#include "AssetUtilWifi.h"

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
               WiFiClass *wifiInstance)
{

    // Explicitly set mode, esp defaults to STA+AP
    wifiInstance->mode(WIFI_STA);
#if defined(ESP32)
    wifiInstance->setHostname(wifiHostName);
#elif defined(ESP8266)
    wifiInstance->hostname(wifiHostName);
#endif

    // Get the secret key from preferences if it is available
    String secretKey = preferences->getString(secretKeyParamName, secretKeyDefaultValue);

    // Set configuration settings for WiFi Manager
    WiFiManagerParameter secretKeyUiParameter(secretKeyParamName, secretKeyParamDisplayName, secretKey.c_str(), 20);
    wifiManager->addParameter(&secretKeyUiParameter);
    wifiManager->setTimeout(WIFI_NETWORK_TIMEOUT_SECONDS);                    // Timeout in seconds
    wifiManager->setConfigPortalTimeout(WIFI_CONFIG_NETWORK_TIMEOUT_SECONDS); // Timeout in seconds
    wifiManager->setMinimumSignalQuality(WIFI_NETWORK_MIN_SIGNAL_QUALITY);    // Signal quality in %
    wifiManager->setAPStaticIPConfig(WIFI_NETWORK_STATIC_IP, WIFI_NETWORK_STATIC_GATEWAY_IP, WIFI_NETWORK_STATIC_SUBNET_MASK);
    // wifiManager->setSTAStaticIPConfig(WIFI_NETWORK_STATIC_IP, WIFI_NETWORK_STATIC_GATEWAY_IP, WIFI_NETWORK_STATIC_SUBNET_MASK);
    wifiManager->setDebugOutput(true);
    // Now, let's connect to the previously saved WiFi, or start the
    // configuration WiFi. This is encapsulated in WifIManager's autoConnect() method.
    if (wifiManager->autoConnect(wifiNetworkName, secretKey.c_str()))
    {
        Serial.println("Connected to WiFi!");
        Serial.println(wifiInstance->SSID());
        Serial.println("IP address: ");
        Serial.println(wifiInstance->localIP());

        // If we are connected to configuration WiFi, we can read the secret key from the UI parameter.
        Serial.println("Getting secret key parameter...");
        Serial.println("Secret key parameter length: ");
        Serial.println(secretKeyUiParameter.getValueLength());
        Serial.println("Saving secret key...");
        if (secretKeyUiParameter.getValueLength() > 0)
        {
            if (secretKey.equals(secretKeyUiParameter.getValue()) == false)
            {
                Serial.println("Secret key changed, saving new value...");
                preferences->putString(secretKeyParamName, secretKeyUiParameter.getValue());
            }
            secretHandler->setSecret(secretIdForSecretsManager, secretKeyUiParameter.getValue(), secretKeyUiParameter.getValueLength());
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