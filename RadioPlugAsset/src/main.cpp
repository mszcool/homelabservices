#include "SecretHandler.h"
#include "AssetUtilWifi.h"
#include <Arduino.h>
#include <Preferences.h>

#if defined(ESP32)
#include <WiFi.h>
#include "SwitchServerEsp32.h"
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include "SwitchServerEsp8266.h"
#endif

#include <DNSServer.h>
#include <WiFiManager.h>

const char *WIFI_HOST_NAME = "mszSwitchServer";
const char *WIFI_NETWORK_NAME = "mszIoTConfigWiFi";
const char *SECRET_KEY_PARAM_NAME = "secretKeyUx";
const char *SECRET_KEY_PARAM_DISPLAYNAME = "Secret Key for Authorization (empty = disabled)";
const char *SECRET_KEY_DEFAULT_VALUE = "yourSecret123!here";
const char *PREFERENCES_NAMESPACE = "msz-radioswsrv";

WiFiManager wifiManager;
Preferences preferences;

MszSecretHandler *secretHandler;
MszSwitchLogic *switchLogic;

#if defined(ESP32)
MszSwitchApiEsp32 switchServer(MszSwitchWebApi::HTTP_AUTH_SECRET_ID, 80);
#elif defined(ESP8266)
MszSwitchApiEsp8266 switchServer(MszSwitchWebApi::HTTP_AUTH_SECRET_ID, 80);
#endif

void setup()
{
  // Start the serial logger
  Serial.begin(9600);
  Serial.println("Starting radio switch server...");

  // Open preferences for the namespace of this app
  preferences.begin(PREFERENCES_NAMESPACE, false);

  // Creating a secrets handler
  secretHandler = new MszSecretHandler();
  switchLogic = new MszSwitchLogic();

  // Next, start the WifiManager
  setupWifi(WIFI_HOST_NAME,
            WIFI_NETWORK_NAME,
            SECRET_KEY_PARAM_NAME,
            SECRET_KEY_PARAM_DISPLAYNAME,
            SECRET_KEY_DEFAULT_VALUE,
            PREFERENCES_NAMESPACE,
            MszSwitchWebApi::HTTP_AUTH_SECRET_ID,
            secretHandler,
            &preferences,
            &wifiManager,
            &WiFi);

  // Now configure the switch logic in the API server before we begin.
  switchServer.configure(switchLogic);

  // After WiFi was set-up, we can configure the web server.
  switchServer.begin(secretHandler);
}

void loop()
{
  // put your main code here, to run repeatedly:
  switchServer.loop();

  // handle RC receive commands
  switchLogic->handleSwitchReceiveData();
}