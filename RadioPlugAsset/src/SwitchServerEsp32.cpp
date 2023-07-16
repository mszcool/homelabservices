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

    // Read the string data from parameters.
    server.arg("name").toCharArray(params.switchName, MAX_SWITCH_NAME_LENGTH + 1);
    server.arg("oncommand").toCharArray(params.switchOnCommand, MAX_SWITCH_COMMAND_LENGTH + 1);
    server.arg("offcommand").toCharArray(params.switchOffCommand, MAX_SWITCH_COMMAND_LENGTH + 1);
    
    // Read types that require conversion from parameters.
    params.switchProtocol = std::stoi(server.arg("protocol").c_str());
    std::istringstream convTri(server.arg("isTriState").c_str());
    convTri >> std::boolalpha >> params.isTriState;
    
    Serial.println("Getting switch data parameters - exit.");
    return params;
}

SwitchMetadataParams MszSwitchApiEsp32::getSwitchMetadataParameters()
{
    Serial.println("Getting switch metadata parameters - enter.");
    SwitchMetadataParams params;
    server.arg("name").toCharArray(params.sensorName, MAX_SENSOR_NAME_LENGTH + 1);
    server.arg("location").toCharArray(params.sensorLocation, MAX_SENSOR_LOCATION_LENGTH + 1);
    Serial.println("Getting switch metadata parameters - exit.");
    return params;
}

#endif