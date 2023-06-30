#include "SwitchServer.h"
#include <functional>

MszSwitchWebApi::MszSwitchWebApi(int port) : server(port) { }

void MszSwitchWebApi::begin() {
  server.on("/switchon", std::bind(&MszSwitchWebApi::handleSwitchOn, this));
  server.on("/switchoff", std::bind(&MszSwitchWebApi::handleSwitchOff, this));
  server.on("/updateswitchdata", std::bind(&MszSwitchWebApi::handleUpdateSwitchData, this));
  server.on("/updatemetadata", std::bind(&MszSwitchWebApi::handleUpdateMetadata, this));
  
  server.begin();
}

CoreHandlerResponse MszSwitchWebApi::handleSwitchOnCore(String switchName) {
  CoreHandlerResponse response;
  response.statusCode = 200;
  response.contentType = "text/plain";
  response.returnContent = "Switch " + switchName + " is now ON";
  return response;
}

CoreHandlerResponse MszSwitchWebApi::handleSwitchOffCore(String switchName) {
  CoreHandlerResponse response;
  response.statusCode = 200;
  response.contentType = "text/plain";
  response.returnContent = "Switch " + switchName + " is now OFF";
  return response;
}

CoreHandlerResponse MszSwitchWebApi::handleUpdateSwitchDataCore() {
  CoreHandlerResponse response;
  response.statusCode = 200;
  response.contentType = "text/plain";
  response.returnContent = "Updating switch metadata...";
  return response;
}

CoreHandlerResponse MszSwitchWebApi::handleUpdateMetadataCore() {
  CoreHandlerResponse response;
  response.statusCode = 200;
  response.contentType = "text/plain";
  response.returnContent = "Updating general metadata...";
  return response;
}

#if defined(ESP8266)
void MszSwitchWebApi::loop() {
  server.handleClient();
}

void MszSwitchWebApi::handleSwitchOn() {
  String switchName = server.arg("name");
  CoreHandlerResponse responseData = handleSwitchOnCore(switchName);
  server.send(responseData.statusCode, responseData.contentType, responseData.returnContent);
}

void MszSwitchWebApi::handleSwitchOff() {
  String switchName = server.arg("name");
  CoreHandlerResponse responseData = handleSwitchOnCore(switchName);
  server.send(responseData.statusCode, responseData.contentType, responseData.returnContent);
}

void MszSwitchWebApi::handleUpdateSwitchData() {
  CoreHandlerResponse responseData = handleUpdateSwitchDataCore();
  server.send(responseData.statusCode, responseData.contentType, responseData.returnContent);
}

void MszSwitchWebApi::handleUpdateMetadata() {
  CoreHandlerResponse responseData = handleUpdateMetadataCore();
  server.send(responseData.statusCode, responseData.contentType, responseData.returnContent);
}

#elif defined(EPS32)
void MyWebServer::loop() {
  server.begin();
}

server.on("/switchon", HTTP_GET, [](AsyncWebServerRequest *request) {
  String switchName = request->arg("name");
  // Add your code to switch on 'switchName'
  request->send(200, "text/plain", "Switch " + switchName + " is now on");
});

server.on("/switchoff", HTTP_GET, [](AsyncWebServerRequest *request) {
  String switchName = request->arg("name");
  // Add your code to switch off 'switchName'
  request->send(200, "text/plain", "Switch " + switchName + " is now off");
});

server.on("/updateswitchdata", HTTP_GET, [](AsyncWebServerRequest *request) {
  // Add your code to update switch data
  request->send(200, "text/plain", "Switch data updated");
});

server.on("/updatemetadata", HTTP_GET, [](AsyncWebServerRequest *request) {
  // Add your code to update meta data
  request->send(200, "text/plain", "Metadata updated");
});
#endif