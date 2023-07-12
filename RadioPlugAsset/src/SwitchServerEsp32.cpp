#if defined(ESP32)

#include <string>
#include <sstream>
#include <iostream>
#include <functional>
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

String MszSwitchApiEsp32::getTokenFromAuthorizationHeader()
{
    String authHeader = server.header("Authorization");
    int firstPipe = authHeader.indexOf('|');
    int secondPipe = authHeader.indexOf('|', firstPipe + 1);

    if (firstPipe == -1 || secondPipe == -1)
    {
        // The authorization header does not contain two pipe characters
        // Return an empty string to indicate an error
        return "";
    }

    // The token is the substring between the two pipe characters
    return authHeader.substring(firstPipe + 1, secondPipe);
}

String MszSwitchApiEsp32::getSignatureFromAuthorizationHeader()
{
    String authHeader = server.header("Authorization");
    int firstPipe = authHeader.indexOf('|');
    int secondPipe = authHeader.indexOf('|', firstPipe + 1);

    if (secondPipe == -1)
    {
        // The authorization header does not contain two pipe characters
        // Return an empty string to indicate an error
        return "";
    }

    // The signature is the substring after the second pipe character
    return authHeader.substring(secondPipe + 1);
}

int MszSwitchApiEsp32::getTimestampFromAuthorizationHeader()
{
    String authHeader = server.header("Authorization");
    int firstPipe = authHeader.indexOf('|');

    if (firstPipe == -1)
    {
        // The authorization header does not contain a pipe character
        // Return 0 to indicate an error
        return 0;
    }

    // The timestamp is the substring before the first pipe character
    String timestampStr = authHeader.substring(0, firstPipe);

    return timestampStr.toInt();
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