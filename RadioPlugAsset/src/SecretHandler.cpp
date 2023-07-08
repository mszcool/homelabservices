#include "SecretHandler.h"
#include <unordered_map>
#include <functional>

#if defined(ESP32)
#include <LittleFS.h>
#include <TimeLib.h>
#include <mbedtls/md.h>
#elif defined(ESP8266)
#include <FS.h>
#endif

MszSecretHandler::MszSecretHandler()
{
    for (int i = 0; i < MszSecretHandler::MAX_SECRETS; i++)
    {
        this->secrets[i] = NULL;
    }
}

MszSecretHandler::~MszSecretHandler()
{
    for (int i = 0; i < MszSecretHandler::MAX_SECRETS; i++)
    {
        if (this->secrets[i] != NULL)
        {
            delete this->secrets[i];
        }
    }
}

String MszSecretHandler::toHexString(const uint8_t *input, size_t length)
{
    char output[length * 2 + 1];
    for (size_t i = 0; i < length; i++)
    {
        sprintf(output + i * 2, "%02x", input[i]);
    }
    return String(output);
}

char* MszSecretHandler::getSecret(int index)
{
    Serial.println("Getting secret - enter.");

    if (index < 0 || index > MszSecretHandler::MAX_SECRETS)
    {
        Serial.println("Getting secret - exit.");
        return NULL;
    }
   
    Serial.println("Secret index:");
    Serial.println(index);
    if (this->secrets[index] != NULL)
    {
        Serial.println("Secret exists, reading in Arduino String.");
        return this->secrets[index];
    }

    Serial.println("Getting secret - exit.");
    return NULL;
}

bool MszSecretHandler::setSecret(int index, const char *secret, int secretLength)
{
    Serial.println("Writing secret - enter.");
    this->secrets[index] = new char[secretLength + 1];
    memccpy(this->secrets[index], secret, 0, secretLength + 1);
    this->secrets[index][secretLength] = '\0';
    Serial.println("Writing secret - exit.");
    return true;
}

#if defined(ESP32)

bool MszSecretHandler::validateTokenSignature(String token, long tokenTimestamp, String secretKey, String signature, int tokenExpirationSeconds)
{
    Serial.println("Validating token signature - enter.");

    String tokenTimestampString = String(tokenTimestamp);

    unsigned char output[32]; // Buffer to hold the HMAC output
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    const size_t keyLength = strlen(secretKey.c_str());
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, (unsigned char *)secretKey.c_str(), keyLength);
    mbedtls_md_hmac_update(&ctx, (unsigned char *)token.c_str(), token.length());
    mbedtls_md_hmac_update(&ctx, (unsigned char *)tokenTimestampString.c_str(), tokenTimestampString.length());
    mbedtls_md_hmac_finish(&ctx, output);
    mbedtls_md_free(&ctx);

    char *expectedSignature = (char *)output;
    String expectedSignatureHex = toHexString(output, sizeof(output));

    bool result = (strcmp(signature.c_str(), expectedSignature) == 0);
    result &= (now() - tokenTimestamp <= tokenExpirationSeconds);

    Serial.println("Validating token signature - exit.");
    return result;
}

#elif defined(ESP8266)

bool MszSecretHandler::validateTokenSignature(String token, long tokenTimestamp, String secretKey, String signature, int tokenExpirationSeconds)
{
    Serial.println("Validating token signature - enter.");
    Serial.println("Validating token signature not implemented for ESP8266, yet... always returning true...");
    Serial.println("Validating token signature - exit.");
    return true;
}

#endif