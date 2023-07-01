#include "SwitchServer.h"
#include <functional>

const char* API_ENDPOINT_INFO = "info";
const char* API_ENDPOINT_ON = "/switchon";
const char* API_ENDPOINT_OFF = "/switchoff";
const char* API_ENDPOINT_UPDATESWITCHDATA = "/updateswitchdata";
const char* API_ENDPOINT_UPDATEINFO = "/updatemetadata";

const char *API_PARAM_SWITCHID = "switchid";
const char *API_PARAM_SWITCHNAME = "switchname";
const char *API_PARAM_SWITCHCOMMAND = "switchcommand";
const char *API_PARAM_ISTRISTATE = "switchistristate";

MszSwitchWebApi::MszSwitchWebApi(int port) {
  this->serverPort = port;
}

void MszSwitchWebApi::begin() {
  this->registerEndpoint(API_ENDPOINT_INFO, std::bind(&MszSwitchWebApi::handleGetInfo, this, std::placeholders::_1));
  this->registerEndpoint(API_ENDPOINT_ON, std::bind(&MszSwitchWebApi::handleSwitchOn, this, std::placeholders::_1));
  this->registerEndpoint(API_ENDPOINT_OFF, std::bind(&MszSwitchWebApi::handleSwitchOff, this, std::placeholders::_1));
  this->registerEndpoint(API_ENDPOINT_UPDATESWITCHDATA, std::bind(&MszSwitchWebApi::handleUpdateSwitchData, this, std::placeholders::_1));
  this->registerEndpoint(API_ENDPOINT_UPDATEINFO, std::bind(&MszSwitchWebApi::handleUpdateMetadata, this, std::placeholders::_1));
  
  this->beginServe();
}

void MszSwitchWebApi::loop() {
  this->beginServe();
}

void MszSwitchWebApi::handleGetInfo(MszSwitchWebApiRequestContext *context) {
  CoreHandlerResponse response = this->handleGetInfoCore();
  return this->sendResponseData(context, response);
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

CoreHandlerResponse MszSwitchWebApi::handleGetInfoCore() {
  CoreHandlerResponse response;
  response.statusCode = 200;
  response.contentType = "text/plain";
  response.returnContent = "Switch API is running";
  return response;
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