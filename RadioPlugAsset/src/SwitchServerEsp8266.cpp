#if defined(ESP8266)

#include <functional>
#include "SwitchServerEsp8266.h"

MszSwitchApiEsp8266::MszSwitchApiEsp8266(short secretId, int serverPort) : MszSwitchWebApi(secretId, serverPort)
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

String MszSwitchApiEsp8266::getQueryStringParam(String paramName)
{
  Serial.println("Getting query string param - enter.");
  String paramValue = server.arg(paramName.c_str());
  Serial.println("Getting query string param - exit.");
  return paramValue;
}

String MszSwitchApiEsp8266::getHttpHeader(String headerName)
{
  Serial.println("Getting HTTP header - enter.");
  String headerValue = server.header(headerName.c_str());
  Serial.println("Getting HTTP header - exit.");
  return headerValue;
}

#endif