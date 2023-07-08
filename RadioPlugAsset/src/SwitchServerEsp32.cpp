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