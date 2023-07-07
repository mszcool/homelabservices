#if defined(ESP32)

#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include <TimeLib.h>
#include <mbedtls/md.h>
#include "SwitchServerEsp32.h"

MszSwitchApiEsp32::MszSwitchApiEsp32(int port)
    : MszSwitchWebApi(port), server(port)
{
}

void MszSwitchApiEsp32::beginServe()
{
    this->server.begin();
}

void MszSwitchApiEsp32::handleClient()
{
    this->server.handleClient();
}

void MszSwitchApiEsp32::registerEndpoint(String endpoint, std::function<void()> handler)
{
    server.on(endpoint.c_str(), HTTP_GET, handler);
}

String MszSwitchApiEsp32::getTokenAuthorizationHeader()
{
    Serial.println("Getting token authorization header - enter.");
    String header = server.header("Authorization");
    Serial.println("Getting token authorization header - exit.");
    return header;
}

String MszSwitchApiEsp32::getTokenSignatureHeader()
{
    Serial.println("Getting token signature header - enter.");
    String header = server.header("Signature");
    Serial.println("Getting token signature header - exit.");
    return header;
}

bool MszSwitchApiEsp32::validateTokenSignature(String token, String signature, String secretKey)
{
    Serial.println("Validating token signature - enter.");
    unsigned char output[32]; // Buffer to hold the HMAC output
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    const size_t keyLength = strlen(secretKey.c_str());
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, (unsigned char *)secretKey.c_str(), keyLength);
    mbedtls_md_hmac_update(&ctx, (unsigned char *)token.c_str(), token.length());
    mbedtls_md_hmac_finish(&ctx, output);
    mbedtls_md_free(&ctx);

    char *expectedSignature = (char *)output;

    // TODO: add time stamp to validation == && now() - tokenTimestamp <= TOKEN_EXPIRATION_SECONDS)
    bool result = (strcmp(signature.c_str(), expectedSignature) == 0); 
    Serial.println("Validating token signature - exit.");
    return result;
}

void MszSwitchApiEsp32::sendResponseData(CoreHandlerResponse response)
{
    Serial.println("Sending response data - enter.");
    server.send(response.statusCode, response.contentType, response.returnContent);
    Serial.println("Sending response data - exit.");
}

String MszSwitchApiEsp32::getSwitchNameParameter()
{
    Serial.println("Getting switch name parameter - enter/exit.");
    return server.arg("name");
}

SwitchDataParams MszSwitchApiEsp32::getSwitchDataParameters()
{
    Serial.println("Getting switch data parameters - enter.");
    SwitchDataParams params;
    params.switchId = std::stoi(server.arg("id").c_str());
    params.switchName = server.arg("name");
    params.switchCommand = server.arg("switchName");
    std::istringstream conv(server.arg("isTriState").c_str());
    conv >> std::boolalpha >> params.isTriState;
    Serial.println("Getting switch data parameters - exit.");
    return params;
}

SwitchMetadataParams MszSwitchApiEsp32::getSwitchMetadataParameters()
{
    Serial.println("Getting switch metadata parameters - enter.");
    SwitchMetadataParams params;
    params.sensorName = server.arg("name");
    params.sensorLocation = server.arg("location");
    params.token = server.arg("token");
    Serial.println("Getting switch metadata parameters - exit.");
    return params;
}

#endif