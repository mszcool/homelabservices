#include <functional>
#include <sstream>
#include <iomanip>

#include "SwitchServer.h"
#include "SecretHandler.h"

/*
 * The base class constructors are doing all the initialization, already.
 */
MszSwitchWebApi::MszSwitchWebApi() : MszAssetApiBase()
{
}

MszSwitchWebApi::MszSwitchWebApi(short secretId, int serverPort) : MszAssetApiBase(secretId, serverPort)
{
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
  this->registerPutEndpoint(API_ENDPOINT_UPDATESWITCHDATA, std::bind(&MszSwitchWebApi::handleUpdateSwitchData, this));
  Serial.println("MszSwitchWebApi::beginCfg() - Switch API endpoints configured!");

  Serial.println("MszSwitchWebApi::beginCfg() - Configuring RCSwitch...");
  switchSender.enableTransmit(RCSWITCH_DATA_PORT);
  switchSender.setPulseLength(RCSWITCH_DATA_PULSE_LENGTH);
  switchSender.setProtocol(RCSWITCH_DATA_PROTOCOL);
  switchSender.setRepeatTransmit(RCSWITCH_REPEAT_TRANSMIT);
  Serial.println("MszSwitchWebApi::beginCfg() - RCSwitch configured!");

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
  performAuthorizedAction([&]() {
    // First, get the switch parameters from the request.
    SwitchDataParams switchData;
    if (!(this->getSwitchDataParams(switchData)))
    {
      Serial.println("MszSwitchWebApi::handleUpdateSwitchDataCore - switch data invalid");

      CoreHandlerResponse response;
      response.statusCode = HTTP_BAD_REQUEST_CODE;
      response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
      response.returnContent = "INVALID_SWITCH_DATA";

      Serial.println("MszSwitchWebApi::handleUpdateSwitchDataCore - exit");
      return response;
    }
    
    // If all parameters are validated, execute the core logic.
    MszSwitchRepository switchRepository;
    bool succeeded = switchRepository.saveSwitchData(switchData.switchName, switchData);

    CoreHandlerResponse response;
    response.statusCode = (succeeded ? HTTP_OK_CODE : HTTP_INTERNAL_SERVER_ERROR_CODE);
    response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
    response.returnContent = (succeeded ? "SWITCH_UPDATED" : "SWITCH_UPDATE_FAILED");
    return response;
  });
  Serial.println("MszSwitchWebApi::handleUpdateSwitchData - exit");
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
    switchSender.setPulseLength(switchData.pulseLength > 0 ? switchData.pulseLength : RCSWITCH_DATA_PULSE_LENGTH);
    switchSender.setRepeatTransmit(switchData.repeatTransmit > 0 ? switchData.repeatTransmit : RCSWITCH_REPEAT_TRANSMIT);
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
