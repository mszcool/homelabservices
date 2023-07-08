#include "SecretHandler.h"
#include <Arduino.h>

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
const int WIFI_NETWORK_TIMEOUT_SECONDS = 300;
const int WIFI_CONFIG_NETWORK_TIMEOUT_SECONDS = 300;
const int WIFI_NETWORK_MIN_SIGNAL_QUALITY = 8;
const IPAddress WIFI_NETWORK_STATIC_IP(10, 0, 1, 99);
const IPAddress WIFI_NETWORK_STATIC_GATEWAY_IP(10, 0, 1, 1);
const IPAddress WIFI_NETWORK_STATIC_SUBNET_MASK(255, 255, 255, 0);
const char *SECRET_KEY_PARAM_NAME = "secretKeyUx";
const char *SECRET_KEY_PARAM_DISPLAYNAME = "Secret Key for Authorization (empty = disabled)";
const char *SECRET_KEY_DEFAULT_VALUE = "yourSecret123!here";

// Todo: put this in a central place.
const int HTTP_AUTH_SECRET_INDEX = 0;

WiFiManager wifiManager;
#if defined(ESP32)
MszSwitchApiEsp32 switchServer(80);
#elif defined(ESP8266)
MszSwitchApiEsp8266 switchServer(80);
#endif

void setupWifi(MszSecretHandler *secretHandler);

void setup()
{
  // Start the serial logger
  Serial.begin(9600);
  Serial.println("Starting radio switch server...");

  // Creating a secrets handler
  MszSecretHandler *secretHandler = new MszSecretHandler();

  // Next, start the WifiManager
  setupWifi(secretHandler);

  // After WiFi was set-up, we can configure the web server.
  switchServer.begin(secretHandler);
}

void loop()
{
  // put your main code here, to run repeatedly:
  switchServer.loop();
}

/// @brief Offers a configuration WiFi or connects to a known network
void setupWifi(MszSecretHandler *secretHandler)
{
  // Explicitly set mode, esp defaults to STA+AP
  WiFi.mode(WIFI_STA);
#if defined(ESP32)
  WiFi.setHostname(WIFI_HOST_NAME);
#elif defined(ESP8266)
  WiFi.hostname(WIFI_HOST_NAME);
#endif

  // Set configuration settings for WiFi Manager
  WiFiManagerParameter secretKeyUiParameter(SECRET_KEY_PARAM_NAME, SECRET_KEY_PARAM_DISPLAYNAME, SECRET_KEY_DEFAULT_VALUE, 20);
  wifiManager.addParameter(&secretKeyUiParameter);
  wifiManager.setTimeout(WIFI_NETWORK_TIMEOUT_SECONDS);                    // Timeout in seconds
  wifiManager.setConfigPortalTimeout(WIFI_CONFIG_NETWORK_TIMEOUT_SECONDS); // Timeout in seconds
  wifiManager.setMinimumSignalQuality(WIFI_NETWORK_MIN_SIGNAL_QUALITY);    // Signal quality in %
  // wifiManager.setSTAStaticIPConfig(WIFI_NETWORK_STATIC_IP, WIFI_NETWORK_STATIC_GATEWAY_IP, WIFI_NETWORK_STATIC_SUBNET_MASK);
  wifiManager.setDebugOutput(true);

  // Now, let's connect to the previously saved WiFi, or start the
  // configuration WiFi. This is encapsulated in WifIManager's autoConnect() method.
  if (wifiManager.autoConnect(WIFI_NETWORK_NAME))
  {
    Serial.println("Connected to WiFi!");
    Serial.println(WiFi.SSID());
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // If we are connected to configuration WiFi, we can read the secret key from the UI parameter.
    Serial.println("Getting secret key parameter...");
    Serial.println("Secret key parameter length: ");
    Serial.println(secretKeyUiParameter.getValueLength());
    Serial.println("Saving secret key...");
    if(secretKeyUiParameter.getValueLength() > 0)
    {
      secretHandler->setSecret(HTTP_AUTH_SECRET_INDEX, secretKeyUiParameter.getValue(), secretKeyUiParameter.getValueLength());
    }
    Serial.println("Secret key saved!");
  }
  else
  {
    Serial.println("Failed to connect to WiFi, timed out with both, previously connected WiFi and the access point WiFi!");
    delay(3000);
    // Reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }
  Serial.println("----------");
}