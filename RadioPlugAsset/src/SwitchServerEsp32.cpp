#if defined(EPS32)

#include <functional>
#include "SwitchServerEsp32.h"

MszSwitchApiEsp32::MszSwitchApiEsp32(int port) : MszSwitchWebApi(port)
{
}

void MszSwitchApiEsp32::beginServe()
{
    server.handleClient();
}

void MszSwitchApiEsp32::registerEndpoint(String endpoint, std::function<void(void)> handler)
{
    server.on(endpoint.c_str(), HTTP_GET, [&handler](AsyncWebServerRequest *request) {
        MszSwitchWebApiEsp32RequestContext context;
        context.request = request;
        handler((MszSwitchWebApiRequestContext*)&context);
    });
}

void MszSwitchApiEsp32::sendResponseData(MszSwitchWebApiRequestContext *context, CoreHandlerResponse response)
{
    MszSwitchWebApiEsp32RequestContext *esp32Context = (MszSwitchWebApiEsp32RequestContext *)context;
    context.request->send(response.statusCode, response.contentType, response.returnContent);
}

String MszSwitchApiEsp32::getSwitchNameParameter(MszSwitchWebApiRequestContext *context)
{
    MszSwitchWebApiEsp32RequestContext *esp32Context = (MszSwitchWebApiEsp32RequestContext *)context;
    return context->request->arg("name");
}

SwitchDataParams MszSwitchApiEsp32::getSwitchDataParameters(MszSwitchWebApiRequestContext *context)
{
    MszSwitchWebApiEsp32RequestContext *esp32Context = (MszSwitchWebApiEsp32RequestContext *)context;

    SwitchDataParams params;
    // params.switchId = server.arg("id");
    //  params.isTriState = server.arg("isTriState");
    params.switchName = context->request->arg("name");
    params.switchCommand context->request->arg("switchName");
    return params;
}

SwitchMetadataParams MszSwitchApiEsp32::getSwitchMetadataParameters(MszSwitchWebApiRequestContext *context)
{
    MszSwitchWebApiEsp32RequestContext *esp32Context = (MszSwitchWebApiEsp32RequestContext *)context;

    SwitchMetadataParams params;
    // params.switchId = server.arg("id");
    params.sensorName = context->request->arg("name");
    params.sensorLocation = context->request->arg("location");
    params.token = context->request->arg("token");
    return params;
}

void MyWebServer::loop()
{
    server.begin();
}

#endif