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

void MszSwitchApiEsp32::registerEndpoint(String endpoint, std::function<void(MszSwitchWebApiRequestContext *)> handler)
{
    server.on(endpoint.c_str(), HTTP_GET, [&handler]() {
        handler(nullptr);
    });
}

void MszSwitchApiEsp32::sendResponseData(MszSwitchWebApiRequestContext *context, CoreHandlerResponse response)
{
    server.send(response.statusCode, response.contentType, response.returnContent);
}

String MszSwitchApiEsp32::getSwitchNameParameter(MszSwitchWebApiRequestContext *context)
{
    return server.arg("name");
}

SwitchDataParams MszSwitchApiEsp32::getSwitchDataParameters(MszSwitchWebApiRequestContext *context)
{
    SwitchDataParams params;
    params.switchId = std::stoi(server.arg("id").c_str());
    params.switchName = server.arg("name");
    params.switchCommand = server.arg("switchName");
    std::istringstream conv(server.arg("isTriState").c_str());
    conv >> std::boolalpha >> params.isTriState;
    return params;
}

SwitchMetadataParams MszSwitchApiEsp32::getSwitchMetadataParameters(MszSwitchWebApiRequestContext *context)
{
    SwitchMetadataParams params;
    params.sensorName = server.arg("name");
    params.sensorLocation = server.arg("location");
    params.token = server.arg("token");
    return params;
}

#endif