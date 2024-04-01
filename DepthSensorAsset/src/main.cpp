#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <TimeLib.h>

#include "AssetUtilWifi.h"
#include "SecretHandler.h"

#include "DepthSensorEntities.h"
#include "DepthSensorRepository.h"
#include "DepthSensorWebApi.h"

const char *WIFI_HOST_NAME = "mszDepthSensor";
const char *WIFI_NETWORK_NAME = "mszIoTConfigWiFi";
const char *SECRET_KEY_PARAM_NAME = "secretKeyUx";
const char *SECRET_KEY_PARAM_DISPLAYNAME = "Secret Key for Authorization (empty = disabled)";
const char *SECRET_KEY_DEFAULT_VALUE = "yourSecret123!here";
const char *PREFERENCES_NAMESPACE = "msz-depthswsrv";

WiFiManager wifiManager;
Preferences preferences;
DepthSensorConfig depthSensorConfig;
MszSecretHandler *secretHandler;
MszDepthSensorRepository *depthRepository;
MszDepthSensorApi *depthSensorApi;

time_t lastMeasurementTime = 0;

float createMeasurement()
{
  // Send the signal. Start with turning off the sensor for a few microseconds to avoid interference.
  // Then send the signal and wait for the response to calculate the distance.
  digitalWrite(ULTRASOUND_SENSOR_SEND_PIN, LOW);
  delay(5);
  digitalWrite(ULTRASOUND_SENSOR_SEND_PIN, HIGH);
  delayMicroseconds(3);
  digitalWrite(ULTRASOUND_SENSOR_SEND_PIN, LOW);

  while (digitalRead(ULTRASOUND_SENSOR_RECEIVE_PIN) == LOW)
  {
    // Do nothing, wait for the pulse to come back.
  }

  unsigned long startTime = micros();

  while (digitalRead(ULTRASOUND_SENSOR_RECEIVE_PIN) == HIGH)
  {
    // Do nothing, wait for the pulse to end.
  }

  unsigned long endTime = micros();

  // Receive the signal on the receiving sensor.
  // replaced due to accuracy issues: int duration = pulseIn(ULTRASOUND_SENSOR_RECEIVE_PIN, HIGH);

  float distanceInCm = ((endTime - startTime) / 2) * ULTRASOUND_CENTIMETERS_PER_MICROSECOND;

  return distanceInCm;
}

void setup() {
  // Start the serial logger
  Serial.begin(9600);
  Serial.println("Starting depth sensor server...");

  // Open preferences for the namespace of this app
  preferences.begin(PREFERENCES_NAMESPACE, false);

  // Creating the required instances of the core implementation objects.
  secretHandler = new MszSecretHandler();
  depthRepository = new MszDepthSensorRepository();
  depthSensorApi = new MszDepthSensorApi(depthRepository, MszDepthSensorApi::HTTP_AUTH_SECRET_ID, 80);

  // Load the settings for the depth sensor
  //depthSensorConfig = depthRepository.loadDepthSensorConfig();
  
  // Next, start the WifiManager
  setupWifi(WIFI_HOST_NAME,
            WIFI_NETWORK_NAME,
            SECRET_KEY_PARAM_NAME,
            SECRET_KEY_PARAM_DISPLAYNAME,
            SECRET_KEY_DEFAULT_VALUE,
            PREFERENCES_NAMESPACE,
            MszDepthSensorApi::HTTP_AUTH_SECRET_ID,
            secretHandler,
            &preferences,
            &wifiManager,
            &WiFi);

  // Configure the time for the sensor to a default
  // time. That way, the sensor measurements work, at least, even if
  // the time is not correctly set. The time can be set by a controller
  // running outside of the sensor instead of introducing a dependency
  // to the Internet for this sensor to work.
  setTime(0, 0, 0, 1, 1, 2024); // Set the time to Jan 1, 2024 (midnight)
  lastMeasurementTime = now();

  // Set the PINs for the Ultrasound sensor.
  pinMode(ULTRASOUND_SENSOR_SEND_PIN, OUTPUT);
  pinMode(ULTRASOUND_SENSOR_RECEIVE_PIN, INPUT);

  // Now start the web server
  depthSensorApi->begin(secretHandler);
}

void loop() {

  // Take a senor measurement, but only per defined interval.
  time_t currentTime = now();
  if ( (currentTime - lastMeasurementTime) >= depthSensorConfig.measureIntervalInSeconds)
  {
    Serial.println("\nTaking a measurement...");
    Serial.println("Last measurement time: " + String(lastMeasurementTime));
    Serial.println("Current time: " + String(currentTime));

    // Loading the updated configuration to apply after the next cycle.
    depthSensorConfig = depthRepository->loadDepthSensorConfig();

    // Create the measurement entity.
    DepthSensorMeasurement depthMeasurement;
    depthMeasurement.measurementTime = now();
    depthMeasurement.hasBeenRetrieved = false;

    // Take a measurement.
    depthMeasurement.measurementInCm = createMeasurement();

    // Print the measurement
    Serial.println("-- Measurement time: " + String(depthMeasurement.measurementTime));
    Serial.println("-- Measurement in cm: " + String(depthMeasurement.measurementInCm));
    
    // Store the measurement in the repository.
    depthRepository->addMeasurement(depthMeasurement);

    // Update the last measurement time
    lastMeasurementTime = currentTime;
  }

  // Then handle the request
  depthSensorApi->loop();
}