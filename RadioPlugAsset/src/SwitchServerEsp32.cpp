#if defined(ESP32)

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
    server.on(endpoint.c_str(), HTTP_GET, [&handler](AsyncWebServerRequest *request) {
        MszSwitchWebApiEsp32RequestContext context(request);
        context.request = request;
        handler((MszSwitchWebApiRequestContext*)&context);
    });
}

void MszSwitchApiEsp32::sendResponseData(MszSwitchWebApiRequestContext *context, CoreHandlerResponse response)
{
    MszSwitchWebApiEsp32RequestContext *esp32Context = (MszSwitchWebApiEsp32RequestContext *)context;
    esp32Context->request->send(response.statusCode, response.contentType, response.returnContent);
}

String MszSwitchApiEsp32::getSwitchNameParameter(MszSwitchWebApiRequestContext *context)
{
    MszSwitchWebApiEsp32RequestContext *esp32Context = (MszSwitchWebApiEsp32RequestContext *)context;
    return esp32Context->request->arg("name");
}

SwitchDataParams MszSwitchApiEsp32::getSwitchDataParameters(MszSwitchWebApiRequestContext *context)
{
    MszSwitchWebApiEsp32RequestContext *esp32Context = (MszSwitchWebApiEsp32RequestContext *)context;

    SwitchDataParams params;
    // params.switchId = server.arg("id");
    //  params.isTriState = server.arg("isTriState");
    params.switchName = esp32Context->request->arg("name");
    params.switchCommand = esp32Context->request->arg("switchName");
    return params;
}

SwitchMetadataParams MszSwitchApiEsp32::getSwitchMetadataParameters(MszSwitchWebApiRequestContext *context)
{
    MszSwitchWebApiEsp32RequestContext *esp32Context = (MszSwitchWebApiEsp32RequestContext *)context;

    SwitchMetadataParams params;
    // params.switchId = server.arg("id");
    params.sensorName = esp32Context->request->arg("name");
    params.sensorLocation = esp32Context->request->arg("location");
    params.token = esp32Context->request->arg("token");
    return params;
}

#endif