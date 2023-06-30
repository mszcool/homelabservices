#include "SwitchServer.h"
#include <functional>

MszSwitchWebApi::MszSwitchWebApi(int port) {
  this->serverPort = port;
}

void MszSwitchWebApi::begin() {
  this->registerEndpoint("/switchon", std::bind(&MszSwitchWebApi::handleSwitchOn, this, std::placeholders::_1));
  this->registerEndpoint("/switchoff", std::bind(&MszSwitchWebApi::handleSwitchOff, this, std::placeholders::_1));
  this->registerEndpoint("/updateswitchdata", std::bind(&MszSwitchWebApi::handleUpdateSwitchData, this, std::placeholders::_1));
  this->registerEndpoint("/updatemetadata", std::bind(&MszSwitchWebApi::handleUpdateMetadata, this, std::placeholders::_1));
  
  this->beginServe();
}

void MszSwitchWebApi::loop() {
  this->beginServe();
}

void MszSwitchWebApi::handleSwitchOn(MszSwitchWebApiRequestContext *context) {
  String switchName = this->getSwitchNameParameter(context);
  CoreHandlerResponse response = this->handleSwitchOnCore(switchName);
  return this->sendResponseData(context, response);
}

void MszSwitchWebApi::handleSwitchOff(MszSwitchWebApiRequestContext *context) {
  String switchName = this->getSwitchNameParameter(context);
  CoreHandlerResponse response = this->handleSwitchOffCore(switchName);
  return this->sendResponseData(context, response);
}

void MszSwitchWebApi::handleUpdateSwitchData(MszSwitchWebApiRequestContext *context) {
  CoreHandlerResponse response = this->handleUpdateSwitchDataCore();
  return this->sendResponseData(context, response);
}

void MszSwitchWebApi::handleUpdateMetadata(MszSwitchWebApiRequestContext *context) {
  CoreHandlerResponse response = this->handleUpdateMetadataCore();
  return this->sendResponseData(context, response);
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