#if defined(ESP8266)

#include "SwitchServer.h"
#include <functional>

MszSwitchApiEsp8266::MszSwitchApiEsp8266(int port) : MszSwitchWebApi(port)
{
}

void MszSwitchApiEsp8266::beginServe()
{
  server.handleClient();
}

void MszSwitchApiEsp8266::registerEndpoint(String endpoint, std::function<void(MszSwitchWebApiRequestContext *)> handler)
{
  server.on(endpoint.c_str(), HTTP_GET, [&handler]()
            { handler(nullptr); });
}

void MszSwitchApiEsp8266::sendResponseData(MszSwitchWebApiRequestContext *context, CoreHandlerResponse response)
{
  server.send(response.statusCode, response.contentType, response.returnContent);
}

String MszSwitchApiEsp8266::getSwitchNameParameter(MszSwitchWebApiRequestContext *context)
{
  return server.arg("name");
}

SwitchDataParams MszSwitchApiEsp8266::getSwitchDataParameters(MszSwitchWebApiRequestContext *context)
{
  SwitchDataParams params;
  // params.switchId = server.arg("id");
  //  params.isTriState = server.arg("isTriState");
  params.switchName = server.arg("name");
  params.switchCommand = server.arg("switchName");
  return params;
}

SwitchMetadataParams MszSwitchApiEsp8266::getSwitchMetadataParameters(MszSwitchWebApiRequestContext *context)
{
  SwitchMetadataParams params;
  // params.switchId = server.arg("id");
  params.sensorName = server.arg("name");
  params.sensorLocation = server.arg("location");
  params.token = server.arg("token");
  return params;
}

#endif