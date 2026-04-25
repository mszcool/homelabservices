#!/usr/bin/env python3
"""Forward pool depth-sensor measurements to MQTT.

Pipeline:
  1. Invoke assetDepthSensor.py to fetch the recent measurement window.
  2. Drop spike outliers using a local Hampel filter (median + MAD over a
     small sliding window). This catches isolated bad readings such as
     ``80, 60, 80`` while leaving normal jitter untouched.
  3. Forward only measurements newer than the last forwarded timestamp
     (persisted in a small state file) so re-runs don't republish.
  4. Publish each remaining measurement to the MQTT topic
     ``/waterlevels/pooltank`` via ``mosquitto_pub``.
  5. If the newest measurement's year differs from the current host year,
     the sensor clock has drifted -- run ``assetDepthSensor.py settime``
     to resync it.

Designed to be invoked from cron via depthForward.sh, which passes all
credentials and tunables explicitly on the command line.

Required CLI arguments:
  --depth-ip                  IP of the depth sensor
  --depth-secret              Shared secret for assetDepthSensor.py
  --mqtt-host                 MQTT broker host
  --mqtt-user                 MQTT username
  --mqtt-password             MQTT password
  --topic                     MQTT topic to publish to
  --state-file                Path to the persisted last-forwarded timestamp
  --outlier-k                 Hampel k threshold (e.g. 3.0)
  --outlier-window            Hampel window radius in samples (e.g. 3 -> 7)
  --outlier-floor-cm          Minimum effective MAD-based threshold in cm

Environment variable:
  LOGGING                     Set to ON for verbose stderr logging.
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from statistics import median


# ---------------------------------------------------------------------------
# Configuration & helpers
# ---------------------------------------------------------------------------

DEPTH_DIR = Path(os.environ.get("HOME", "/root")) / "depth"
DEPTH_PYTHON = DEPTH_DIR / "pyenv" / "bin" / "python"
DEPTH_SENSOR_SCRIPT = DEPTH_DIR / "assetDepthSensor.py"

TIME_FORMAT = "%Y-%m-%d %H:%M:%S"


def log(msg: str) -> None:
    """Print to stderr when LOGGING=ON. Stderr keeps stdout clean for cron."""
    if os.environ.get("LOGGING") == "ON":
        print(msg, file=sys.stderr)


# ---------------------------------------------------------------------------
# Sensor invocation
# ---------------------------------------------------------------------------

def _python_for_sensor() -> str:
    """Use the depth/pyenv interpreter when present, else system python3."""
    if DEPTH_PYTHON.exists():
        return str(DEPTH_PYTHON)
    return sys.executable or "python3"


def _run_sensor(args: list[str], depth_ip: str, depth_secret: str) -> str:
    """Invoke assetDepthSensor.py with the given subcommand args.

    Returns stdout (string). Raises SystemExit on non-zero exit code.
    LOGGING is forced off in the child so stdout is clean JSON only.
    """
    cmd = [
        _python_for_sensor(),
        str(DEPTH_SENSOR_SCRIPT),
        "--ip", depth_ip,
        "--secret", depth_secret,
        *args,
    ]
    log(f"[sensor] running: {' '.join(cmd[:1] + cmd[1:2] + ['--ip', '<redacted>', '--secret', '<redacted>'] + args)}")
    child_env = os.environ.copy()
    child_env["LOGGING"] = "OFF"
    proc = subprocess.run(
        cmd,
        capture_output=True,
        text=True,
        env=child_env,
        check=False,
    )
    if proc.returncode != 0:
        print(
            f"ERROR: assetDepthSensor.py {args[0]} failed (rc={proc.returncode}): "
            f"{proc.stderr.strip()}",
            file=sys.stderr,
        )
        sys.exit(proc.returncode or 1)
    return proc.stdout


def fetch_measurements(depth_ip: str, depth_secret: str) -> list[dict]:
    raw = _run_sensor(["measurements"], depth_ip, depth_secret)
    try:
        payload = json.loads(raw)
    except json.JSONDecodeError as exc:
        print(f"ERROR: cannot parse sensor JSON: {exc}", file=sys.stderr)
        print(raw, file=sys.stderr)
        sys.exit(3)
    measurements = payload.get("measurements", [])
    # Sort ascending by measureTime to make windowed filtering meaningful and
    # to ensure stable state-file updates regardless of sensor ordering.
    measurements.sort(key=lambda m: m["measureTime"])
    log(f"[sensor] fetched {len(measurements)} measurements")
    return measurements


def set_sensor_time(depth_ip: str, depth_secret: str) -> None:
    log("[sensor] year drift detected -- calling settime")
    _run_sensor(["settime"], depth_ip, depth_secret)


# ---------------------------------------------------------------------------
# Outlier filtering
# ---------------------------------------------------------------------------
#
# Algorithm: local Hampel filter.
#
#   For each point i, look at a window of `2*radius + 1` neighbours
#   centred on i. Compute the median m and the median absolute deviation
#   MAD = median(|x_j - m|). Scaled by 1.4826, MAD is a robust estimate
#   of sigma for normally-distributed data. A point is an outlier when
#       |x_i - m| > k * 1.4826 * MAD
#
#   Why Hampel and not z-score?
#     - mean/stddev are themselves dragged by the outlier (a single 60 cm
#       reading shifts the mean by several cm), making z-score unreliable
#       on small samples.
#     - median/MAD have a 50% breakdown point: up to half the window can
#       be contaminated before the estimate moves at all. Perfect for
#       isolated spikes like the user's "80, 60, 80" example.
#
#   Edge handling: at the start/end of the array there are fewer than
#   `2*radius + 1` neighbours; we use whatever is available (asymmetric
#   window) so the first/last samples still get evaluated.
#
#   MAD-zero floor: when neighbouring measurements are essentially
#   identical (very still water), MAD collapses to 0 and any tiny jitter
#   would be flagged. We therefore enforce a minimum effective threshold
#   (`outlier_floor_cm`, default 2 cm) below which a deviation is never
#   considered an outlier.

MAD_SIGMA = 1.4826


def _local_threshold(window_values: list[float], k: float, floor_cm: float) -> tuple[float, float]:
    m = median(window_values)
    mad = median(abs(v - m) for v in window_values)
    threshold = max(k * MAD_SIGMA * mad, floor_cm)
    return m, threshold


def filter_outliers(
    measurements: list[dict],
    *,
    radius: int = 3,
    k: float = 3.0,
    floor_cm: float = 2.0,
) -> tuple[list[dict], list[dict]]:
    """Return (kept, dropped) measurement lists."""
    if len(measurements) < 3:
        return list(measurements), []

    values = [float(m["centimeters"]) for m in measurements]
    kept: list[dict] = []
    dropped: list[dict] = []
    for i, m in enumerate(measurements):
        lo = max(0, i - radius)
        hi = min(len(values), i + radius + 1)
        # Build the comparison window EXCLUDING the candidate itself so a
        # genuine spike cannot influence its own median/MAD.
        window = values[lo:i] + values[i + 1:hi]
        if not window:
            kept.append(m)
            continue
        local_median, threshold = _local_threshold(window, k, floor_cm)
        if abs(values[i] - local_median) > threshold:
            log(
                f"[outlier] dropping {m['measureTime']} = {values[i]:.3f} cm "
                f"(local median {local_median:.3f}, threshold {threshold:.3f})"
            )
            dropped.append(m)
        else:
            kept.append(m)
    return kept, dropped


# ---------------------------------------------------------------------------
# State file (last forwarded timestamp)
# ---------------------------------------------------------------------------

def read_last_forwarded(path: Path) -> str | None:
    try:
        value = path.read_text(encoding="utf-8").strip()
    except FileNotFoundError:
        return None
    return value or None


def write_last_forwarded(path: Path, value: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    # Atomic-ish: write to temp then rename so a crash cannot leave a
    # half-written timestamp that would be parsed back as garbage.
    tmp = path.with_suffix(path.suffix + ".tmp")
    tmp.write_text(value + "\n", encoding="utf-8")
    tmp.replace(path)


# ---------------------------------------------------------------------------
# MQTT publishing
# ---------------------------------------------------------------------------

def publish_measurement(
    *,
    host: str,
    user: str,
    password: str,
    topic: str,
    measurement: dict,
) -> None:
    payload = json.dumps({
        "centimeters": measurement["centimeters"],
        "measureTime": measurement["measureTime"],
    })
    cmd = [
        "mosquitto_pub",
        "-h", host,
        "-u", user,
        "-P", password,
        "-t", topic,
        "-m", payload,
    ]
    proc = subprocess.run(cmd, capture_output=True, text=True, check=False)
    if proc.returncode != 0:
        print(
            f"ERROR: mosquitto_pub failed (rc={proc.returncode}) for "
            f"{measurement['measureTime']}: {proc.stderr.strip()}",
            file=sys.stderr,
        )
        # Re-raise via SystemExit so the state file is NOT advanced past
        # an unpublished measurement; the next run will retry it.
        sys.exit(proc.returncode or 1)
    log(f"[mqtt] published {topic} <- {payload}")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Forward pool depth-sensor measurements to MQTT.",
    )
    parser.add_argument("--depth-ip", required=True,
                        help="IP address of the depth sensor.")
    parser.add_argument("--depth-secret", required=True,
                        help="Shared secret for assetDepthSensor.py.")
    parser.add_argument("--mqtt-host", required=True,
                        help="MQTT broker hostname or IP.")
    parser.add_argument("--mqtt-user", required=True,
                        help="MQTT username.")
    parser.add_argument("--mqtt-password", required=True,
                        help="MQTT password.")
    parser.add_argument("--topic", required=True,
                        help="MQTT topic to publish each measurement to.")
    parser.add_argument("--state-file", required=True,
                        help="Path to the file persisting the last forwarded timestamp.")
    parser.add_argument("--outlier-k", required=True, type=float,
                        help="Hampel k threshold (e.g. 3.0).")
    parser.add_argument("--outlier-window", required=True, type=int,
                        help="Hampel window radius in samples (e.g. 3 -> total window of 7).")
    parser.add_argument("--outlier-floor-cm", required=True, type=float,
                        help="Minimum effective MAD-based threshold in cm to avoid flagging normal jitter.")
    return parser.parse_args(argv)


def main() -> int:
    args = parse_args()

    depth_ip = args.depth_ip
    depth_secret = args.depth_secret
    mqtt_user = args.mqtt_user
    mqtt_pwd = args.mqtt_password
    mqtt_host = args.mqtt_host
    topic = args.topic
    state_file = Path(args.state_file)
    k = args.outlier_k
    radius = args.outlier_window
    floor_cm = args.outlier_floor_cm

    measurements = fetch_measurements(depth_ip, depth_secret)
    if not measurements:
        log("[main] no measurements returned; nothing to do")
        return 0

    # --- Sensor clock drift check --------------------------------------------
    # Use the newest measurement timestamp as the sensor's view of "now".
    # If its year doesn't match the host's current year, the sensor clock
    # is off (typical after a power loss) and must be resynced.
    newest = measurements[-1]["measureTime"]
    try:
        newest_year = datetime.strptime(newest, TIME_FORMAT).year
    except ValueError:
        print(f"ERROR: unparseable measureTime from sensor: {newest!r}",
              file=sys.stderr)
        return 4
    host_year = datetime.now().year
    if newest_year != host_year:
        log(f"[main] sensor year {newest_year} != host year {host_year}")
        set_sensor_time(depth_ip, depth_secret)
        # Don't forward anything this run -- the timestamps we just got
        # are from the wrong epoch and would pollute the time series.
        return 0

    # --- Outlier filtering ---------------------------------------------------
    kept, dropped = filter_outliers(
        measurements, radius=radius, k=k, floor_cm=floor_cm,
    )
    log(f"[main] kept {len(kept)} / dropped {len(dropped)} measurements")

    # --- Time-window filtering -----------------------------------------------
    last_forwarded = read_last_forwarded(state_file)
    if last_forwarded:
        log(f"[main] last forwarded timestamp: {last_forwarded}")
        to_forward = [m for m in kept if m["measureTime"] > last_forwarded]
    else:
        log("[main] no state file; forwarding everything kept")
        to_forward = list(kept)

    if not to_forward:
        log("[main] no new measurements to forward")
        return 0

    # --- Publish -------------------------------------------------------------
    for m in to_forward:
        publish_measurement(
            host=mqtt_host,
            user=mqtt_user,
            password=mqtt_pwd,
            topic=topic,
            measurement=m,
        )

    # Advance state to the newest timestamp we *attempted to publish*.
    # If publish_measurement failed we'd have exited already, so reaching
    # here means every entry in to_forward was published successfully.
    write_last_forwarded(state_file, to_forward[-1]["measureTime"])
    log(f"[main] state advanced to {to_forward[-1]['measureTime']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
