#ifndef DEPTHSENSORENTITIES
#define DEPTHSENSORENTITIES

#include <Arduino.h>

#define ULTRASOUND_METERS_PER_SECOND 343.2
#define ULTRASOUND_CENTIMETERS_PER_MILLISECOND (ULTRASOUND_METERS_PER_SECOND * 100 / 1000)
#define ULTRASOUND_CENTIMETERS_PER_MICROSECOND (ULTRASOUND_CENTIMETERS_PER_MILLISECOND / 1000)

#define ULTRASOUND_SENSOR_SEND_PIN 23
#define ULTRASOUND_SENSOR_RECEIVE_PIN 22

// Maximum time to wait for the ultrasonic echo before giving up on a single
// measurement. This prevents the busy-wait loops from blocking forever if the
// echo pulse never returns (e.g. sensor fault or object out of range).
// ~30 ms corresponds to a round-trip distance well beyond the sensor's usable
// range (> 5 m), so a valid reading will always complete before this elapses.
#define ULTRASOUND_MEASUREMENT_TIMEOUT_MICROS 30000UL

// Sentinel returned by createMeasurement() when no valid echo was received
// within ULTRASOUND_MEASUREMENT_TIMEOUT_MICROS.
#define ULTRASOUND_MEASUREMENT_INVALID -1.0f

#define MIN_MEASURE_INTERVAL_IN_SECONDS 1
#define DEFAULT_MEASURE_INTERVAL_IN_SECONDS (60 * 5)
#define MAX_MEASURE_INTERVAL_IN_SECONDS 32767

#define MIN_MEASUREMENTS_TO_KEEP_UNTIL_PURGE 10
#define MAX_MEASUREMENTS_TO_KEEP_UNTIL_PURGE 100

/// @brief Configuration settings for the Depth Sensor
/// @details Defines the interval in seconds between measurements and the number of measurements to keep before purging.
struct DepthSensorConfig {
    bool isDefault;
    int measureIntervalInSeconds;
    int measurementsToKeepUntilPurge;
};

/// @brief Measurement data for the Depth Sensor
/// @details Defines the time of the measurement, the measurement in centimeters, and whether the measurement has been retrieved.
struct DepthSensorMeasurement {
    unsigned long measurementTime;
    float measurementInCm;
    bool hasBeenRetrieved;
};

/// @brief State of the Depth Sensor with a maximum number of measurements
/// @details Defines the measurements that have been taken by the Depth Sensor.
struct DepthSensorState {
    // Measurement caching items. These are not stored to the filesystem
    // to avoid stressing the sensors flash memory too much. Increases lifetime.
    int measurementCount;
    DepthSensorMeasurement measurements[MAX_MEASUREMENTS_TO_KEEP_UNTIL_PURGE];

    // Configuration management to avoid reading configuration from file if nothing has changed.
    DepthSensorConfig currentConfig;
    time_t lastConfigTimeRead;
    time_t lastConfigTimeWrite;
};

#endif // DEPTHSENSORENTITIES