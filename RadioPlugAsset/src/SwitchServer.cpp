#include <functional>
#include <sstream>
#include <iomanip>

#include "SwitchServer.h"
#include "SecretHandler.h"

/*
 * The base class constructors and public initialization methods are doing all the initialization, already.
 */
MszSwitchWebApi::MszSwitchWebApi() : MszAssetApiBase()
{
}

MszSwitchWebApi::MszSwitchWebApi(short secretId, int serverPort) : MszAssetApiBase(secretId, serverPort)
{
}

void MszSwitchWebApi::configure(MszSwitchLogic *switchLogicInstance)
{
  this->switchLogic = switchLogicInstance;
}

/*
 * Main execution functions - begin and loop
 */

void MszSwitchWebApi::beginCfg()
{
  Serial.println("MszSwitchWebApi::beginCfg() - enter");

  Serial.println("MszSwitchWebApi::beginCfg() - Configuring Switch API secret handler");
  this->registerPutEndpoint(API_ENDPOINT_ON, std::bind(&MszSwitchWebApi::handleSwitchOn, this));
  this->registerPutEndpoint(API_ENDPOINT_OFF, std::bind(&MszSwitchWebApi::handleSwitchOff, this));
  this->registerPutEndpoint(API_ENDPOINT_UPDATESWITCHRECEIVE, std::bind(&MszSwitchWebApi::handleUpdateSwitchReceive, this));
  this->registerPutEndpoint(API_ENDPOINT_UPDATESWITCHDATA, std::bind(&MszSwitchWebApi::handleUpdateSwitchData, this));
  Serial.println("MszSwitchWebApi::beginCfg() - Switch API endpoints configured!");

  // If the switch logic is not present, throw an exception
  if (this->switchLogic == nullptr)
  {
    Serial.println("MszSwitchWebApi::beginCfg() - Switch logic not configured!");
    throw std::runtime_error("Switch logic not configured!");
  }

  Serial.println("MszSwitchWebApi::beginCfg() - exit");
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
  Serial.println("MszSwitchWebApi::handleUpdateSwitchData - enter");
  performAuthorizedAction([&]()
                          {
    // First, get the switch parameters from the request.
    SwitchDataParams switchData;
    if (!(this->getSwitchDataParams(switchData)))
    {
      Serial.println("MszSwitchWebApi::handleUpdateSwitchDataCore - switch data invalid");

      CoreHandlerResponse response;
      response.statusCode = HTTP_BAD_REQUEST_CODE;
      response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;
      response.returnContent = this->getErrorJsonDocument(
                                      HTTP_BAD_REQUEST_CODE,
                                      "Invalid Switch Data!",
                                      "You did not provide valid switch data for updating the switch!");

      Serial.println("MszSwitchWebApi::handleUpdateSwitchDataCore - exit");
      return response;
    }
    
    // If all parameters are validated, execute the core logic.
    MszSwitchRepository switchRepository;
    bool succeeded = switchRepository.saveSwitchData(switchData.switchName, switchData);

    CoreHandlerResponse response;
    response.statusCode = (succeeded ? HTTP_OK_CODE : HTTP_INTERNAL_SERVER_ERROR_CODE);
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;

    JsonDocument respDoc;
    respDoc["switchName"] = switchData.switchName;
    respDoc["switchStatus"] = (succeeded ? "SWITCH_UPDATED" : "SWITCH_UPDATE_FAILED");
    serializeJsonPretty(respDoc, response.returnContent);
    
    return response; });
  Serial.println("MszSwitchWebApi::handleUpdateSwitchData - exit");
}

void MszSwitchWebApi::handleUpdateSwitchReceive()
{
  Serial.println("MszSwitchWebApi::handleUpdateSwitchReceive - enter");
  performAuthorizedAction([&]()
                          {
    // First, get the switch parameters from the request.
    SwitchReceiveParams receiveParams;
    if (!(this->getSwitchReceiveParams(receiveParams)))
    {
      Serial.println("MszSwitchWebApi::handleUpdateSwitchReceiveCore - switch receive data invalid");

      CoreHandlerResponse response;
      response.statusCode = HTTP_BAD_REQUEST_CODE;
      response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;
      response.returnContent = this->getErrorJsonDocument(
                                      HTTP_BAD_REQUEST_CODE,
                                      "Invalid Switch Receive Data!",
                                      "You did not provide valid switch receive data for updating the switch!");

      Serial.println("MszSwitchWebApi::handleUpdateSwitchReceiveCore - exit");
      return response;
    }
    
    // If all parameters are validated, execute the core logic.
    MszSwitchRepository switchRepository;
    std::unordered_map<int, SwitchReceiveParams> receiveData = switchRepository.loadSwitchReceiveData();
    receiveData[receiveParams.switchReceiveDecimalValue] = receiveParams;
    bool succeeded = switchRepository.saveSwitchReceiveData(receiveData);

    CoreHandlerResponse response;
    response.statusCode = (succeeded ? HTTP_OK_CODE : HTTP_INTERNAL_SERVER_ERROR_CODE);
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;

    JsonDocument respDoc;
    respDoc["switchReceiveValue"] = receiveParams.switchReceiveDecimalValue;
    respDoc["switchStatus"] = (succeeded ? "SWITCH_RECEIVE_UPDATED" : "SWITCH_RECEIVE_UPDATE_FAILED");
    serializeJsonPretty(respDoc, response.returnContent);
    
    return response; });
  Serial.println("MszSwitchWebApi::handleUpdateSwitchReceive - exit");
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
  String paramPulseLength = this->getQueryStringParam(MszSwitchWebApi::PARAM_PULSELENGTH);
  String paramRepeatTransmit = this->getQueryStringParam(MszSwitchWebApi::PARAM_REPEATTRANSMIT);

  if (paramSwitchName == nullptr || paramCommandOn == nullptr ||
      paramCommandOff == nullptr || paramProtocol == nullptr ||
      paramIsTriState == nullptr || paramPulseLength == nullptr ||
      paramRepeatTransmit == nullptr)
  {
    Serial.println("Getting switch data parameters - missing parameters - exit.");
    return false;
  }

  if (paramSwitchName == "" || paramCommandOn == "" ||
      paramCommandOff == "" || paramProtocol == "" ||
      paramIsTriState == "" || paramPulseLength == "" ||
      paramRepeatTransmit == "")
  {
    Serial.println("Getting switch data parameters - missing parameters - exit.");
    return false;
  }

  // We need to test, if the pulseLength and the

  // Move the items into the structure
  paramSwitchName.toCharArray(switchParams.switchName, MAX_SWITCH_NAME_LENGTH + 1);
  paramCommandOn.toCharArray(switchParams.switchOnCommand, MAX_SWITCH_COMMAND_LENGTH + 1);
  paramCommandOff.toCharArray(switchParams.switchOffCommand, MAX_SWITCH_COMMAND_LENGTH + 1);
  switchParams.pulseLength = paramPulseLength.toInt();
  switchParams.repeatTransmit = paramRepeatTransmit.toInt();

  // Read types that require conversion from parameters - protocol.
  std::istringstream convProto(paramProtocol.c_str());
  convProto >> std::noskipws >> switchParams.switchProtocol;
  if (convProto.fail())
  {
    Serial.println("Getting switch data parameters - convert protocol failed - exit.");
    return false;
  }
  convProto.clear();
  convProto.str(paramPulseLength.c_str());
  convProto >> std::noskipws >> switchParams.pulseLength;
  if (convProto.fail())
  {
    Serial.println("Getting switch data parameters - convert pulseLength failed - exit.");
    return false;
  }
  convProto.clear();
  convProto.str(paramRepeatTransmit.c_str());
  convProto >> std::noskipws >> switchParams.repeatTransmit;
  if (convProto.fail())
  {
    Serial.println("Getting switch data parameters - convert repeatTransmit failed - exit.");
    return false;
  }

  // Read types that require conversion from parameters - isTriState.
  convProto.clear();
  convProto.str(paramIsTriState.c_str());
  convProto >> std::boolalpha >> switchParams.isTriState;
  if (convProto.fail())
  {
    Serial.println("Getting switch data parameters - convert Tristate failed - exit.");
    return false;
  }

  Serial.println("Getting switch data parameters - exit.");
  return true;
}

bool MszSwitchWebApi::getSwitchReceiveParams(SwitchReceiveParams &receiveParams)
{
  Serial.println("Getting switch receive parameters - enter.");

  // Read the string data from parameters.
  String paramReceiveValue = this->getQueryStringParam(MszSwitchWebApi::PARAM_RECEIVE_VALUE);
  String paramReceiveProtocol = this->getQueryStringParam(MszSwitchWebApi::PARAM_RECEIVE_PROTOCOL);
  String paramReceiveTopic = this->getQueryStringParam(MszSwitchWebApi::PARAM_RECEIVE_TOPIC);
  String paramReceiveCommand = this->getQueryStringParam(MszSwitchWebApi::PARAM_RECEIVE_COMMAND);

  if (paramReceiveValue == nullptr || paramReceiveProtocol == nullptr || paramReceiveCommand == nullptr || paramReceiveTopic == nullptr)
  {
    Serial.println("Getting switch receive parameters - missing parameters - exit.");
    return false;
  }

  if (paramReceiveValue == "" || paramReceiveProtocol == "" || paramReceiveCommand == "" || paramReceiveTopic == "")
  {
    Serial.println("Getting switch receive parameters - missing parameters - exit.");
    return false;
  }

  // Move the items into the structure
  receiveParams.switchReceiveDecimalValue = paramReceiveValue.toInt();
  paramReceiveCommand.toCharArray(receiveParams.switchCommand, MAX_SWITCH_COMMAND_LENGTH + 1);
  paramReceiveTopic.toCharArray(receiveParams.switchTopic, MAX_SWITCH_MQTT_TOPIC_LENGTH + 1);

  // Read types that require conversion from parameters - protocol.
  std::istringstream convProto(paramReceiveProtocol.c_str());
  convProto >> std::noskipws >> receiveParams.switchProtocol;
  if (convProto.fail())
  {
    Serial.println("Getting switch receive parameters - convert protocol failed - exit.");
    return false;
  }

  Serial.println("Getting switch receive parameters - exit.");
  return true;
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
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;
    response.returnContent = this->getErrorJsonDocument(
        HTTP_BAD_REQUEST_CODE,
        "Invalid Switch!",
        "You did not provide a switch name for turning on or off!");

    Serial.println("Switch API handleSwitchOnOffCore - exit");
    return response;
  }

  // If validation succeeded, let's executed the business logic.
  MszSwitchRepository switchRepository;
  CoreHandlerResponse response;

  int switchSucceeded = this->switchLogic->toggleSwitch(switchName, switchItOn);
  if (switchSucceeded)
  {
    Serial.println("Switch API handleSwitchOnOffCore - switch not found");

    response.statusCode = HTTP_NOT_FOUND_CODE;
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;
    response.returnContent = this->getErrorJsonDocument(
        HTTP_NOT_FOUND_CODE,
        "Switch not found!",
        "Switch " + switchName + " cannot be found!");
  }
  else
  {
    Serial.println("Preparing response data...");
    response.statusCode = HTTP_OK_CODE;
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;

    JsonDocument respDoc;
    respDoc["switchName"] = switchName;
    respDoc["switchStatus"] = (switchItOn ? "ON" : "OFF");
    serializeJsonPretty(respDoc, response.returnContent);
  }

  Serial.println("Switch API handleSwitchOnOffCore - exit");
  return response;
}
