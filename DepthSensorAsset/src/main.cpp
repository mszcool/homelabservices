#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <Preferences.h>

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
MszDepthSensorRepository depthRepository;
MszDepthSensorApi depthSensorApi(&depthRepository, MszDepthSensorApi::HTTP_AUTH_SECRET_ID, 80);

void setup() {
  // Start the serial logger
  Serial.begin(9600);
  Serial.println("Starting depth sensor server...");

  // Open preferences for the namespace of this app
  preferences.begin(PREFERENCES_NAMESPACE, false);

  // Creating a secrets handler and repository
  MszSecretHandler *secretHandler = new MszSecretHandler();
  
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

  // Now start the web server
  depthSensorApi.begin(secretHandler);
}

void loop() {
  // First configure the time for the sensor

  // Take a senor measurement

  // Then handle the request
  depthSensorApi.loop();
}