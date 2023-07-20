#include <functional>
#include <sstream>
#include <iomanip>

#include "SwitchServer.h"
#include "SecretHandler.h"

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
    String authHeader = this->getHttpHeader(MszSwitchWebApi::HEADER_AUTHORIZATION);
    int firstPipe = authHeader.indexOf('|');
    int secondPipe = authHeader.indexOf('|', firstPipe + 1);

    if (firstPipe == -1 || secondPipe == -1)
    {
      // The authorization header does not contain two pipe characters
      // Return an empty string to indicate an error
      Serial.println("Switch API authorize FAILED - Invalid authorization token format - exit");
      authZResult = false;
    }
    else
    {
      // The timestamp is the substring before the first pipe character
      String timestampStr = authHeader.substring(0, firstPipe);
      String token = authHeader.substring(firstPipe + 1, secondPipe);
      String signature = authHeader.substring(secondPipe + 1);
      Serial.println("Switch API authorize - token: " + token);
      Serial.println("Switch API authorize - signature: " + signature);
      if (token == nullptr || token == "" || signature == nullptr || signature == "" || timestampStr == nullptr || timestampStr == "")
      {
        Serial.println("Switch API authorize FAILED NO TOKEN - exit");
        authZResult = false;
      }
      else
      {
        // First, convert the timestamp to an int, if possible
        int timestamp = 0;
        std::istringstream issTimeStamp(timestampStr.c_str());
        issTimeStamp >> std::noskipws >> timestamp;
        if (issTimeStamp.eof() || !issTimeStamp.fail())
        {
          authZResult = this->validateAuthorizationToken(timestamp, token, signature);
        }
        else
        {
          Serial.println("Switch API authorize FAILED - Invalid timestamp - exit");
          authZResult = false;
        }
      }
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
                          { return this->handleSwitchOnOffCore(true); });
  Serial.println("Switch API handleSwitchOn - exit");
}

void MszSwitchWebApi::handleSwitchOff()
{
  Serial.println("Switch API handleSwitchOff - enter");
  performAuthorizedAction([&]()
                          { return this->handleSwitchOnOffCore(false); });
  Serial.println("Switch API handleSwitchOff - exit");
}

void MszSwitchWebApi::handleUpdateSwitchData()
{
  Serial.println("Switch API handleUpdateSwitchData - enter");
  performAuthorizedAction([&]()
                          { return this->handleUpdateSwitchDataCore(); });
  Serial.println("Switch API handleUpdateSwitchData - exit");
}

void MszSwitchWebApi::handleUpdateMetadata()
{
  Serial.println("Switch API handleUpdateMetadata - enter");
  performAuthorizedAction([&]()
                          { return this->handleUpdateMetadataCore(); });
  Serial.println("Switch API handleUpdateMetadata - exit");
}

/*
 * Parameter handling functions where needed.
 */

bool MszSwitchWebApi::getSwitchDataParams(SwitchDataParams &switchParams)
{
  Serial.println("Getting switch data parameters - enter.");

  // Read the string data from parameters.
  String paramSwitchName = this->getQueryStringParam(MszSwitchWebApi::PARAM_SWITCH_NAME);
  String paramCommandOn = this->getQueryStringParam(MszSwitchWebApi::PARAM_COMMAND_ON);
  String paramCommandOff = this->getQueryStringParam(MszSwitchWebApi::PARAM_COMMAND_OFF);
  String paramProtocol = this->getQueryStringParam(MszSwitchWebApi::PARAM_PROTOCOL);
  String paramIsTriState = this->getQueryStringParam(MszSwitchWebApi::PARAM_IS_TRISTATE);

  if (paramSwitchName == nullptr || paramCommandOn == nullptr || paramCommandOff == nullptr || paramProtocol == nullptr || paramIsTriState == nullptr)
  {
    Serial.println("Getting switch data parameters - missing parameters - exit.");
    return false;
  }

  if (paramSwitchName == "" || paramCommandOn == "" || paramCommandOff == "" || paramProtocol == "" || paramIsTriState == "")
  {
    Serial.println("Getting switch data parameters - missing parameters - exit.");
    return false;
  }

  // Move the items into the structure
  paramSwitchName.toCharArray(switchParams.switchName, MAX_SWITCH_NAME_LENGTH + 1);
  paramCommandOn.toCharArray(switchParams.switchOnCommand, MAX_SWITCH_COMMAND_LENGTH + 1);
  paramCommandOff.toCharArray(switchParams.switchOffCommand, MAX_SWITCH_COMMAND_LENGTH + 1);

  // Read types that require conversion from parameters - protocol.
  std::istringstream convProto(paramProtocol.c_str());
  convProto >> std::noskipws >> switchParams.switchProtocol;
  if (convProto.fail())
  {
    Serial.println("Getting switch data parameters - convert protocol failed - exit.");
    return false;
  }

  // Read types that require conversion from parameters - isTriState.
  std::istringstream convTri(paramIsTriState.c_str());
  convTri >> std::boolalpha >> switchParams.isTriState;
  if (convTri.fail())
  {
    Serial.println("Getting switch data parameters - convert Tristate failed - exit.");
    return false;
  }
  
  Serial.println("Getting switch data parameters - exit.");
  return true;
}

bool MszSwitchWebApi::getMetadataParams(SwitchMetadataParams &metadataParams)
{
  Serial.println("Getting switch metadata parameters - enter.");

  // Read the string data from parameters.
  String paramSensorName = this->getQueryStringParam(MszSwitchWebApi::PARAM_SENSOR_NAME);
  String paramSensorLocation = this->getQueryStringParam(MszSwitchWebApi::PARAM_SENSOR_LOCATION);

  if (paramSensorName == nullptr || paramSensorLocation == nullptr)
  {
    Serial.println("Getting switch metadata parameters - failed - exit.");
    return false;
  }

  if (paramSensorName == "" || paramSensorLocation == "")
  {
    Serial.println("Getting switch metadata parameters - failed - exit.");
    return false;
  }

  // Now copy data into the structure
  paramSensorName.toCharArray(metadataParams.sensorName, MAX_SENSOR_NAME_LENGTH + 1);
  paramSensorLocation.toCharArray(metadataParams.sensorLocation, MAX_SENSOR_LOCATION_LENGTH + 1);

  Serial.println("Getting switch metadata parameters - exit.");
  return true;
}

/*
 * Authorization related functions, implementation
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

/*
 * Core handler methods with execution logic for web API requests.
 * These methods contain the main business logic, while the handlers above
 * are just wrappers to call these methods next to the concrete implementation methods.
 */

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

CoreHandlerResponse MszSwitchWebApi::handleSwitchOnOffCore(bool switchItOn)
{
  Serial.println("Switch API handleSwitchOnOffCore - enter");

  // Get and validate the parameters
  String switchName = this->getQueryStringParam(MszSwitchWebApi::PARAM_SWITCH_NAME);
  if ((switchName == nullptr) || (switchName == "") || (switchName.length() > MAX_SWITCH_NAME_LENGTH))
  {
    Serial.println("Switch API handleSwitchOnOffCore - switch name not found");

    CoreHandlerResponse response;
    response.statusCode = HTTP_BAD_REQUEST_CODE;
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
    response.returnContent = "Switch name not found!";

    Serial.println("Switch API handleSwitchOnOffCore - exit");
    return response;
  }

  // If validation succeeded, let's executed the business logic.
  MszSwitchRepository switchRepository;
  CoreHandlerResponse response;
  SwitchDataParams switchData = switchRepository.loadSwitchData(switchName);
  if (strnlen(switchData.switchName, MAX_SWITCH_NAME_LENGTH) == 0)
  {
    Serial.println("Switch API handleSwitchOnOffCore - switch not found");

    response.statusCode = HTTP_NOT_FOUND_CODE;
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
    response.returnContent = "Switch " + switchName + " cannot be found!";
  }
  else
  {
    switchSender.setProtocol(switchData.switchProtocol);
    if (switchData.isTriState)
    {
      switchSender.sendTriState((switchItOn ? switchData.switchOnCommand : switchData.switchOffCommand));
    }
    else
    {
      switchSender.send((switchItOn ? switchData.switchOnCommand : switchData.switchOffCommand));
    }

    Serial.println("Preparing response data...");
    response.statusCode = HTTP_OK_CODE;
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
    response.returnContent = "Switch " + switchName + " is now " + (switchItOn ? "ON" : "OFF") + "!";
  }

  Serial.println("Switch API handleSwitchOnOffCore - exit");
  return response;
}

CoreHandlerResponse MszSwitchWebApi::handleUpdateSwitchDataCore()
{
  Serial.println("Switch API handleUpdateSwitchDataCore - enter");

  // First, get the switch parameters from the request.
  SwitchDataParams switchData;
  if (!(this->getSwitchDataParams(switchData)))
  {
    Serial.println("Switch API handleUpdateSwitchDataCore - switch data invalid");

    CoreHandlerResponse response;
    response.statusCode = HTTP_BAD_REQUEST_CODE;
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
    response.returnContent = "Switch data invalid!";

    Serial.println("Switch API handleUpdateSwitchDataCore - exit");
    return response;
  }
  
  // If all parameters are validated, execute the core logic.
  MszSwitchRepository switchRepository;
  bool succeeded = switchRepository.saveSwitchData(switchData.switchName, switchData);

  CoreHandlerResponse response;
  response.statusCode = (succeeded ? HTTP_OK_CODE : HTTP_INTERNAL_SERVER_ERROR_CODE);
  response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
  response.returnContent = (succeeded ? "Switch data updated" : "Switch data update failed");

  Serial.println("Switch API handleUpdateSwitchDataCore - exit");
  return response;
}

CoreHandlerResponse MszSwitchWebApi::handleUpdateMetadataCore()
{
  Serial.println("Switch API handleUpdateMetadataCore - enter");

  // First get the data and validate it.
  SwitchMetadataParams metadata;
  if (!(this->getMetadataParams(metadata)))
  {
    Serial.println("Switch API handleUpdateMetadataCore - metadata invalid");

    CoreHandlerResponse response;
    response.statusCode = HTTP_BAD_REQUEST_CODE;
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
    response.returnContent = "Metadata invalid!";

    Serial.println("Switch API handleUpdateMetadataCore - exit");
    return response;
  }

  // Once validation succeeded, execute the request.
  MszSwitchRepository switchRepository;
  bool succeeded = switchRepository.saveMetadata(metadata);

  CoreHandlerResponse response;
  response.statusCode = (succeeded ? HTTP_OK_CODE : HTTP_INTERNAL_SERVER_ERROR_CODE);
  response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
  response.returnContent = (succeeded ? "Metadata updated" : "Metadata update failed");

  Serial.println("Switch API handleUpdateMetadataCore - exit");
  return response;
}
