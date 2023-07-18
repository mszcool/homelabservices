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

String MszSwitchApiEsp32::getQueryStringParam(String paramName)
{
    return server.arg(paramName.c_str());
}

String MszSwitchApiEsp32::getHttpHeader(String headerName)
{
    return server.header(headerName.c_str());
}

void MszSwitchApiEsp32::sendResponseData(CoreHandlerResponse response)
{
    Serial.println("Sending response data - enter.");
    server.send(response.statusCode, response.contentType, response.returnContent);
    Serial.println("Sending response data - exit.");
}

#endif