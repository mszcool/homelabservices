#include "SwitchServer.h"
#include "SecretHandler.h"
#include <functional>

MszSwitchWebApi::MszSwitchWebApi(int port)
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
    String token = this->getTokenAuthorizationHeader();
    String signature = this->getTokenSignatureHeader();
    if (token == nullptr || token == "" || signature == nullptr || signature == "")
    {
      Serial.println("Switch API authorize FAILED NO TOKEN - exit");
      authZResult = false;
    }
    else
    {
      authZResult = this->validateAuthorizationToken(token, signature);
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
    return this->handleSwitchOnCore(switchName);
  });
  Serial.println("Switch API handleSwitchOn - exit");
}

void MszSwitchWebApi::handleSwitchOff()
{
  Serial.println("Switch API handleSwitchOff - enter");
  performAuthorizedAction([&]()
  {
    String switchName = this->getSwitchNameParameter();
    return this->handleSwitchOffCore(switchName);
  });
  Serial.println("Switch API handleSwitchOff - exit");
}

void MszSwitchWebApi::handleUpdateSwitchData()
{
  Serial.println("Switch API handleUpdateSwitchData - enter");
  performAuthorizedAction([&]()
  {
    return this->handleUpdateSwitchDataCore();
  });
  Serial.println("Switch API handleUpdateSwitchData - exit");
}

void MszSwitchWebApi::handleUpdateMetadata()
{
  Serial.println("Switch API handleUpdateMetadata - enter");
  performAuthorizedAction([&]()
  {
    return this->handleUpdateMetadataCore();
  });
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
  char* mySecret = this->secretHandler->getSecret(MszSwitchWebApi::HTTP_AUTH_SECRET_ID);
  if (mySecret == NULL)
  {
    Serial.println("Switch API validateAuthorizationToken not activate because of empty secret - exit");
    return true;
  }

  // Now, let's use the secret to validate the token
  // Split the token into the timestamp and the rest of the token
  int delimiterPosition = token.indexOf("|");
  long tokenTimestamp = token.substring(0, delimiterPosition).toInt();
  String tokenBody = token.substring(delimiterPosition + 1);

  // TODO: Add timestamp to validation
  String secretKey = String(mySecret);
  bool validationResult = this->secretHandler->validateTokenSignature(tokenBody, tokenTimestamp, secretKey, signature, TOKEN_EXPIRATION_SECONDS);
  Serial.println("Switch API validateAuthorizationToken - exit");
  return validationResult;
}

void MszSwitchWebApi::performAuthorizedAction(std::function<CoreHandlerResponse()> action)
{
  CoreHandlerResponse response;
  if (!this->authorize())
  {
    Serial.println("Switch API - authorization failed");
    response.statusCode = 401;
    response.contentType = "text/plain";
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