#!/bin/bash
# Wrapper: forward depth-sensor measurements to MQTT (water level topic).
#
# Reads credentials from the environment (same convention as
# depthMeasure.sh / poolStatus.sh) and passes everything explicitly to
# depthForward.py, which now requires all parameters as CLI flags.
#
# Required env: depthIp, depthSecret, mqttUser, mqttPwd
# Optional env (with defaults): mqttHost, depthForwardTopic,
#   depthForwardStateFile, depthForwardOutlierK,
#   depthForwardOutlierWindow, depthForwardOutlierFloor
#
# Run from cron, e.g. every 5 minutes:
#   */5 * * * * /root/depthForward.sh

set -eu

: "${depthIp:?depthIp is required}"
: "${depthSecret:?depthSecret is required}"
: "${mqttUser:?mqttUser is required}"
: "${mqttPwd:?mqttPwd is required}"

mqttHost="${mqttHost:-192.168.99.218}"
topic="${depthForwardTopic:-waterlevels/pooltank}"
stateFile="${depthForwardStateFile:-$HOME/depth/depthForward.state}"
outlierK="${depthForwardOutlierK:-3.0}"
outlierWindow="${depthForwardOutlierWindow:-3}"
outlierFloor="${depthForwardOutlierFloor:-2.0}"

$HOME/depth/pyenv/bin/python depthForward.py \
    --depth-ip "$depthIp" \
    --depth-secret "$depthSecret" \
    --mqtt-host "$mqttHost" \
    --mqtt-user "$mqttUser" \
    --mqtt-password "$mqttPwd" \
    --topic "$topic" \
    --state-file "$stateFile" \
    --outlier-k "$outlierK" \
    --outlier-window "$outlierWindow" \
    --outlier-floor-cm "$outlierFloor" \
    "$@"
