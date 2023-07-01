#if defined(ESP8266)

#include <functional>
#include "SwitchServerEsp8266.h"

MszSwitchApiEsp8266::MszSwitchApiEsp8266(int port) : MszSwitchWebApi(port)
{
}

void MszSwitchApiEsp8266::beginServe()
{
  server.begin();
}

void MszSwitchApiEsp8266::handleClient()
{
  server.handleClient();
}

void MszSwitchApiEsp8266::registerEndpoint(String endpoint, std::function<void()> handler)
{
  Serial.println("registerEndpoint - enter");
  server.on(endpoint.c_str(), HTTP_GET, handler);
  Serial.println("registerEndpoint - exit");
}

void MszSwitchApiEsp8266::sendResponseData(CoreHandlerResponse response)
{
  Serial.println("Sending response data sendResponseData - enter.");
  server.send(response.statusCode, response.contentType, response.returnContent);
  Serial.println("Sending response data sendResponseData - exit.");
}

String MszSwitchApiEsp8266::getSwitchNameParameter()
{
  Serial.println("Getting switch name parameter - enter/exit.");
  return server.arg("name");
}

SwitchDataParams MszSwitchApiEsp8266::getSwitchDataParameters()
{
  Serial.println("Getting switch data parameters - enter.");
  SwitchDataParams params;
  // params.switchId = server.arg("id");
  //  params.isTriState = server.arg("isTriState");
  params.switchName = server.arg("name");
  params.switchCommand = server.arg("switchName");
  Serial.println("Getting switch data parameters - exit.");
  return params;
}

SwitchMetadataParams MszSwitchApiEsp8266::getSwitchMetadataParameters()
{
  Serial.println("Getting switch metadata parameters - enter.");
  SwitchMetadataParams params;
  // params.switchId = server.arg("id");
  params.sensorName = server.arg("name");
  params.sensorLocation = server.arg("location");
  params.token = server.arg("token");
  Serial.println("Getting switch metadata parameters - exit.");
  return params;
}

#endif