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

String MszSwitchApiEsp8266::getTokenFromAuthorizationHeader()
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

String MszSwitchApiEsp8266::getSignatureFromAuthorizationHeader()
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

int MszSwitchApiEsp8266::getTimestampFromAuthorizationHeader()
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
  // TODO: populate once having an ESP8266 as test device.
  // params.isTriState = false;
  // params.switchCommand = server.arg("command");
  // params.switchName = server.arg("name");
  Serial.println("Getting switch data parameters - exit.");
  return params;
}

SwitchMetadataParams MszSwitchApiEsp8266::getSwitchMetadataParameters()
{
  Serial.println("Getting switch metadata parameters - enter.");
  SwitchMetadataParams params;
  // TODO: populate once having an ESP8266 as test device.
  // params.switchId = server.arg("id");
  // params.sensorName = server.arg("name");
  // params.sensorLocation = server.arg("location");
  Serial.println("Getting switch metadata parameters - exit.");
  return params;
}

#endif