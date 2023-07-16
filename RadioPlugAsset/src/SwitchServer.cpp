#include "SwitchServer.h"
#include "SecretHandler.h"
#include <functional>

MszSwitchWebApi::MszSwitchWebApi(int port)
  : switchSender()
{
  this->serverPort = port;
}

/*
 * Main execution functions - begin and loop
 */

void MszSwitchWebApi::begin(MszSecretHandler *secretHandler)
{
  Serial.println("Configuring Switch API secret handler");
  this->secretHandler = secretHandler;

  Serial.println("Creating RCSwitch() and MszSwitchRepository()...");

  Serial.println("Configuring Switch API endpoints");
  this->registerEndpoint(API_ENDPOINT_INFO, std::bind(&MszSwitchWebApi::handleGetInfo, this));
  this->registerEndpoint(API_ENDPOINT_ON, std::bind(&MszSwitchWebApi::handleSwitchOn, this));
  this->registerEndpoint(API_ENDPOINT_OFF, std::bind(&MszSwitchWebApi::handleSwitchOff, this));
  this->registerEndpoint(API_ENDPOINT_UPDATESWITCHDATA, std::bind(&MszSwitchWebApi::handleUpdateSwitchData, this));
  this->registerEndpoint(API_ENDPOINT_UPDATEINFO, std::bind(&MszSwitchWebApi::handleUpdateMetadata, this));
  Serial.println("Switch API endpoints configured!");

  Serial.println("Configuring Switch API switch sender");
  switchSender.enableTransmit(RCSWITCH_DATA_PORT);
  switchSender.setPulseLength(RCSWITCH_DATA_PULSE_LENGTH);
  switchSender.setProtocol(RCSWITCH_DATA_PROTOCOL);
  switchSender.setRepeatTransmit(RCSWITCH_REPEAT_TRANSMIT);
  Serial.println("Switch API switch sender configured!");

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
  bool authZResult = false;

  Serial.println("Switch API authorize - enter");

  Serial.println("Checking if authorization is enabled...");
  char *secretResponse = this->secretHandler->getSecret(MszSwitchWebApi::HTTP_AUTH_SECRET_ID);
  if (secretResponse == NULL)
  {
    Serial.println("No Secret found, authorization disabled!");
    authZResult = true;
  }
  else
  {
    int timestamp = this->getTimestampFromAuthorizationHeader();
    String token = this->getTokenFromAuthorizationHeader();
    String signature = this->getSignatureFromAuthorizationHeader();
    Serial.println("Switch API authorize - token: " + token);
    Serial.println("Switch API authorize - signature: " + signature);
    if (token == nullptr || token == "" || signature == nullptr || signature == "" || timestamp <= 0)
    {
      Serial.println("Switch API authorize FAILED NO TOKEN - exit");
      authZResult = false;
    }
    else
    {
      authZResult = this->validateAuthorizationToken(timestamp, token, signature);
    }
  }
  Serial.println("Switch API authorize - exit");
  return authZResult;
}

void MszSwitchWebApi::handleGetInfo()
{
  Serial.println("Switch API handleGetInfo - enter");
  performAuthorizedAction([&]()
                          { return this->handleGetInfoCore(); });
  Serial.println("Switch API handleGetInfo - exit");
}

void MszSwitchWebApi::handleSwitchOn()
{
  Serial.println("Switch API handleSwitchOn - enter");
  performAuthorizedAction([&]()
                          {
    String switchName = this->getSwitchNameParameter();
    return this->handleSwitchOnCore(switchName); });
  Serial.println("Switch API handleSwitchOn - exit");
}

void MszSwitchWebApi::handleSwitchOff()
{
  Serial.println("Switch API handleSwitchOff - enter");
  performAuthorizedAction([&]()
                          {
    String switchName = this->getSwitchNameParameter();
    return this->handleSwitchOffCore(switchName); });
  Serial.println("Switch API handleSwitchOff - exit");
}

void MszSwitchWebApi::handleUpdateSwitchData()
{
  Serial.println("Switch API handleUpdateSwitchData - enter");
  performAuthorizedAction([&]()
                          {
                            SwitchDataParams switchParams = this->getSwitchDataParameters(); 
                            return this->handleUpdateSwitchDataCore(switchParams); });
  Serial.println("Switch API handleUpdateSwitchData - exit");
}

void MszSwitchWebApi::handleUpdateMetadata()
{
  Serial.println("Switch API handleUpdateMetadata - enter");
  performAuthorizedAction([&]()
                          { 
                            SwitchMetadataParams metadataParams = this->getSwitchMetadataParameters();
                            return this->handleUpdateMetadataCore(metadataParams); });
  Serial.println("Switch API handleUpdateMetadata - exit");
}

/*
 * Core handler methods with execution logic for web API requests.
 * These methods contain the main business logic, while the handlers above
 * are just wrappers to call these methods next to the concrete implementation methods.
 */

bool MszSwitchWebApi::validateAuthorizationToken(int timestamp, String token, String signature)
{
  Serial.println("Switch API validateAuthorizationToken - enter");

  // First, get the secret
  char *mySecret = this->secretHandler->getSecret(MszSwitchWebApi::HTTP_AUTH_SECRET_ID);
  if (mySecret == NULL)
  {
    Serial.println("Switch API validateAuthorizationToken not activate because of empty secret - exit");
    return true;
  }

  String secretKey = String(mySecret);
  bool validationResult = this->secretHandler->validateTokenSignature(token, timestamp, HTTP_AUTH_SECRET_ID, signature, TOKEN_EXPIRATION_SECONDS);
  Serial.println("Switch API validateAuthorizationToken - exit");
  return validationResult;
}

void MszSwitchWebApi::performAuthorizedAction(std::function<CoreHandlerResponse()> action)
{
  CoreHandlerResponse response;
  if (!this->authorize())
  {
    Serial.println("Switch API - authorization failed");
    response.statusCode = HTTP_UNAUTHORIZED_CODE;
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
    response.returnContent = "Unauthorized";
    this->sendResponseData(response);
  }
  else
  {
    response = action();
    this->sendResponseData(response);
  }
}

CoreHandlerResponse MszSwitchWebApi::handleGetInfoCore()
{
  MszSwitchRepository switchRepository;
  SwitchMetadataParams metadata = switchRepository.loadMetadata();

  CoreHandlerResponse response;
  response.statusCode = HTTP_OK_CODE;
  response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
  response.returnContent = "status=running";
  response.returnContent += "\n\r";
  response.returnContent += "sensorName=" + String(metadata.sensorName);
  response.returnContent += "\n\r";
  response.returnContent += "sensorLocation=" + String(metadata.sensorLocation);

  return response;
}

CoreHandlerResponse MszSwitchWebApi::handleSwitchOnCore(String switchName)
{
  Serial.println("Switch API handleSwitchOnCore - enter");

  MszSwitchRepository switchRepository;
  CoreHandlerResponse response;
  SwitchDataParams switchData = switchRepository.loadSwitchData(switchName);
  if (strnlen(switchData.switchName, MAX_SWITCH_NAME_LENGTH) == 0)
  {
    Serial.println("Switch API handleSwitchOnCore - switch not found");

    response.statusCode = HTTP_NOT_FOUND_CODE;
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
    response.returnContent = "Switch " + switchName + " cannot be found!";
  }
  else
  {
    Serial.println("Switch should be ON, by now!");

    switchSender.setProtocol(switchData.switchProtocol);
    if (switchData.isTriState)
    {
      switchSender.sendTriState(switchData.switchOnCommand);
    }
    else
    {
      switchSender.send(switchData.switchOnCommand);
    }

    Serial.println("Preparing response data...");
    response.statusCode = HTTP_OK_CODE;
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
    response.returnContent = "Switch " + switchName + " is now ON";
  }

  Serial.println("Switch API handleSwitchOnCore - exit");
  return response;
}

CoreHandlerResponse MszSwitchWebApi::handleSwitchOffCore(String switchName)
{
  Serial.println("Switch API handleSwitchOffCore - enter");

  MszSwitchRepository switchRepository;
  CoreHandlerResponse response;
  SwitchDataParams switchData = switchRepository.loadSwitchData(switchName);
  if (strnlen(switchData.switchName, MAX_SWITCH_NAME_LENGTH) == 0)
  {
    Serial.println("Switch API handleSwitchOffCore - switch not found");

    response.statusCode = HTTP_NOT_FOUND_CODE;
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
    response.returnContent = "Switch " + switchName + " cannot be found!";
  }
  else
  {
    Serial.println("Switch should be OFF, by now!");

    switchSender.setProtocol(switchData.switchProtocol);
    if (switchData.isTriState)
    {
      switchSender.sendTriState(switchData.switchOffCommand);
    }
    else
    {
      switchSender.send(switchData.switchOffCommand);
    }

    Serial.println("Preparing response data...");
    response.statusCode = HTTP_OK_CODE;
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
    response.returnContent = "Switch " + switchName + " is now OFF";
  }

  Serial.println("Switch API handleSwitchOffCore - exit");
  return response;
}

CoreHandlerResponse MszSwitchWebApi::handleUpdateSwitchDataCore(SwitchDataParams switchData)
{
  Serial.println("Switch API handleUpdateSwitchDataCore - enter");

  MszSwitchRepository switchRepository;
  bool succeeded = switchRepository.saveSwitchData(switchData.switchName, switchData);

  CoreHandlerResponse response;
  response.statusCode = (succeeded ? HTTP_OK_CODE : HTTP_INTERNAL_SERVER_ERROR_CODE);
  response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
  response.returnContent = (succeeded ? "Switch data updated" : "Switch data update failed");

  Serial.println("Switch API handleUpdateSwitchDataCore - exit");
  return response;
}

CoreHandlerResponse MszSwitchWebApi::handleUpdateMetadataCore(SwitchMetadataParams metadata)
{
  Serial.println("Switch API handleUpdateMetadataCore - enter");

  MszSwitchRepository switchRepository;
  bool succeeded = switchRepository.saveMetadata(metadata);

  CoreHandlerResponse response;
  response.statusCode = (succeeded ? HTTP_OK_CODE : HTTP_INTERNAL_SERVER_ERROR_CODE);
  response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
  response.returnContent = (succeeded ? "Metadata updated" : "Metadata update failed");

  Serial.println("Switch API handleUpdateMetadataCore - exit");
  return response;
}