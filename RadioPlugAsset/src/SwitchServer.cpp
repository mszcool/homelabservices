#include "SwitchServer.h"
#include "SecretHandler.h"
#include <functional>

const char *API_ENDPOINT_INFO = "/info";
const char *API_ENDPOINT_ON = "/switchon";
const char *API_ENDPOINT_OFF = "/switchoff";
const char *API_ENDPOINT_UPDATESWITCHDATA = "/updateswitchdata";
const char *API_ENDPOINT_UPDATEINFO = "/updatemetadata";

const char *API_PARAM_SWITCHID = "switchid";
const char *API_PARAM_SWITCHNAME = "switchname";
const char *API_PARAM_SWITCHCOMMAND = "switchcommand";
const char *API_PARAM_ISTRISTATE = "switchistristate";

const char *HTTP_AUTH_SECRET_NAME = "switchapitoken";
const int TOKEN_EXPIRATION_SECONDS = 60;

MszSwitchWebApi::MszSwitchWebApi(int port)
{
  this->serverPort = port;
}

/*
 * Main execution functions - begin and loop
 */

void MszSwitchWebApi::begin()
{
  Serial.println("Configuring Switch API endpoints");
  this->registerEndpoint(API_ENDPOINT_INFO, std::bind(&MszSwitchWebApi::handleGetInfo, this));
  this->registerEndpoint(API_ENDPOINT_ON, std::bind(&MszSwitchWebApi::handleSwitchOn, this));
  this->registerEndpoint(API_ENDPOINT_OFF, std::bind(&MszSwitchWebApi::handleSwitchOff, this));
  this->registerEndpoint(API_ENDPOINT_UPDATESWITCHDATA, std::bind(&MszSwitchWebApi::handleUpdateSwitchData, this));
  this->registerEndpoint(API_ENDPOINT_UPDATEINFO, std::bind(&MszSwitchWebApi::handleUpdateMetadata, this));
  Serial.println("Switch API endpoints configured!");

  Serial.println("Starting Switch API...");
  this->beginServe();
  Serial.println("Switch API started!");
}

void MszSwitchWebApi::loop()
{
  if (!this->logLoopDone)
  {
    Serial.println("Switch API loop");
    this->logLoopDone = true;
  }
  this->handleClient();
}

/*
 * Handler methods for web API requests.
 */

bool MszSwitchWebApi::authorize()
{
  Serial.println("Switch API authorize - enter");
  String token = this->getTokenAuthorizationHeader();
  String signature = this->getTokenSignatureHeader();
  if (token == nullptr || token == "")
  {
    Serial.println("Switch API authorize FAILED NO TOKEN - exit");
    return false;
  }
  else
  {
    bool result = this->validateAuthorizationToken(token, signature);
    Serial.println("Switch API authorize - exit");
    return result;
  }
}

void MszSwitchWebApi::handleGetInfo()
{
  Serial.println("Switch API handleGetInfo - enter");
  CoreHandlerResponse response = this->handleGetInfoCore();
  this->sendResponseData(response);
  Serial.println("Switch API handleGetInfo - exit");
}

void MszSwitchWebApi::handleSwitchOn()
{
  Serial.println("Switch API handleSwitchOn - enter");
  String switchName = this->getSwitchNameParameter();
  CoreHandlerResponse response = this->handleSwitchOnCore(switchName);
  this->sendResponseData(response);
  Serial.println("Switch API handleSwitchOn - exit");
}

void MszSwitchWebApi::handleSwitchOff()
{
  Serial.println("Switch API handleSwitchOff - enter");
  String switchName = this->getSwitchNameParameter();
  CoreHandlerResponse response = this->handleSwitchOffCore(switchName);
  this->sendResponseData(response);
  Serial.println("Switch API handleSwitchOff - exit");
}

void MszSwitchWebApi::handleUpdateSwitchData()
{
  Serial.println("Switch API handleUpdateSwitchData - enter");
  CoreHandlerResponse response = this->handleUpdateSwitchDataCore();
  this->sendResponseData(response);
  Serial.println("Switch API handleUpdateSwitchData - exit");
}

void MszSwitchWebApi::handleUpdateMetadata()
{
  Serial.println("Switch API handleUpdateMetadata - enter");
  CoreHandlerResponse response = this->handleUpdateMetadataCore();
  return this->sendResponseData(response);
  Serial.println("Switch API handleUpdateMetadata - exit");
}

/*
 * Core handler methods with execution logic for web API requests.
 * These methods contain the main business logic, while the handlers above
 * are just wrappers to call these methods next to the concrete implementation methods.
 */

bool MszSwitchWebApi::validateAuthorizationToken(String token, String signature)
{
  Serial.println("Switch API validateAuthorizationToken - enter");

  // First, get the secret
  MszSecretHandler secretHandler = MszSecretHandler();
  String mySecret = secretHandler.getSecret(HTTP_AUTH_SECRET_NAME);
  if (mySecret == nullptr || mySecret == "")
  {
    Serial.println("Switch API validateAuthorizationToken FAILED NO SECRET - exit");
    return false;
  }

  // Now, let's use the secret to validate the token
  // Split the token into the timestamp and the rest of the token
  int delimiterPosition = token.indexOf("|");
  long tokenTimestamp = token.substring(0, delimiterPosition).toInt();
  String tokenBody = token.substring(delimiterPosition + 1);

  // TODO: Add timestamp to validation
  if (this->validateTokenSignature(tokenBody, signature, mySecret))
  {
    // The signatures match and the token is not expired, so the request is authorized
    return true;
  }
  else
  {
    // The signatures do not match or the token is expired, so the request is not authorized
    return false;
  }

  Serial.println("Switch API validateAuthorizationToken - exit");
  return true;
}

CoreHandlerResponse MszSwitchWebApi::handleGetInfoCore()
{
  CoreHandlerResponse response;
  response.statusCode = 200;
  response.contentType = "text/plain";
  response.returnContent = "Switch API is running";
  return response;
}

CoreHandlerResponse MszSwitchWebApi::handleSwitchOnCore(String switchName)
{
  CoreHandlerResponse response;
  response.statusCode = 200;
  response.contentType = "text/plain";
  response.returnContent = "Switch " + switchName + " is now ON";
  return response;
}

CoreHandlerResponse MszSwitchWebApi::handleSwitchOffCore(String switchName)
{
  CoreHandlerResponse response;
  response.statusCode = 200;
  response.contentType = "text/plain";
  response.returnContent = "Switch " + switchName + " is now OFF";
  return response;
}

CoreHandlerResponse MszSwitchWebApi::handleUpdateSwitchDataCore()
{
  CoreHandlerResponse response;
  response.statusCode = 200;
  response.contentType = "text/plain";
  response.returnContent = "Updating switch metadata...";
  return response;
}

CoreHandlerResponse MszSwitchWebApi::handleUpdateMetadataCore()
{
  CoreHandlerResponse response;
  response.statusCode = 200;
  response.contentType = "text/plain";
  response.returnContent = "Updating general metadata...";
  return response;
}