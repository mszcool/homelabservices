#ifndef MSZ_ASSETUTILWIFI_H
#define MSZ_ASSETUTILWIFI_H

#include "SecretHandler.h"
#include <Arduino.h>
#include <Preferences.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <DNSServer.h>
#include <WiFiManager.h>

const int WIFI_NETWORK_TIMEOUT_SECONDS = 300;
const int WIFI_CONFIG_NETWORK_TIMEOUT_SECONDS = 300;
const int WIFI_NETWORK_MIN_SIGNAL_QUALITY = 8;
const IPAddress WIFI_NETWORK_STATIC_IP(10, 0, 1, 99);
const IPAddress WIFI_NETWORK_STATIC_GATEWAY_IP(10, 0, 1, 1);
const IPAddress WIFI_NETWORK_STATIC_SUBNET_MASK(255, 255, 255, 0);

void setupWifi(const char* wifiHostName,
               const char* wifiNetworkName,
               const char* secretKeyParamName,
               const char* secretKeyParamDisplayName,
               const char* secretKeyDefaultValue,
               const char* preferencesNamespace,
               const int secretIdForSecretsManager,
               MszSecretHandler *secretHandler,
               Preferences *preferences,
               WiFiManager *wifiManager,
               WiFiClass *wifiInstance);

#endif //MSZ_ASSETUTILWIFI_H