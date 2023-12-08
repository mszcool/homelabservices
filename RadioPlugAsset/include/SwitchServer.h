#ifndef MSZ_SWITCHSERVER_H
#define MSZ_SWITCHSERVER_H

#include <Arduino.h>
#include <RCSwitch.h>
#include <AssetApiBase.h>
#include <SecretHandler.h>
#include "SwitchData.h"
#include "SwitchRepository.h"

/// @class MszSwitchWebApi
/// @brief Switch Web Server class handling on/off requests.
/// @details This class handles the web server which is used to turn on and off radio switches.
class MszSwitchWebApi
: public MszAssetApiBase
{
public:
  MszSwitchWebApi();
  MszSwitchWebApi(short secretId, int serverPort);

  static constexpr const char *API_ENDPOINT_ON = "/switchon";
  static constexpr const char *API_ENDPOINT_OFF = "/switchoff";
  static constexpr const char *API_ENDPOINT_UPDATESWITCHDATA = "/updateswitchdata";

  static constexpr const char *API_PARAM_SWITCHID = "switchid";
  static constexpr const char *API_PARAM_SWITCHNAME = "switchname";
  static constexpr const char *API_PARAM_SWITCHCOMMAND = "switchcommand";
  static constexpr const char *API_PARAM_ISTRISTATE = "switchistristate";

  static constexpr const char *PARAM_SWITCH_NAME = "name";
  static constexpr const char *PARAM_COMMAND_ON = "oncommand";
  static constexpr const char *PARAM_COMMAND_OFF = "offcommand";
  static constexpr const char *PARAM_IS_TRISTATE = "istristate";
  static constexpr const char *PARAM_PROTOCOL = "protocol";

  static const int HTTP_AUTH_SECRET_ID = 0;
  static const int TOKEN_EXPIRATION_SECONDS = 60;

  static const int RCSWITCH_DATA_PORT = 23;
  static const int RCSWITCH_DATA_PULSE_LENGTH = 512;
  static const int RCSWITCH_DATA_PROTOCOL = 5;
  static const int RCSWITCH_REPEAT_TRANSMIT = 10;
  static const int RCSWITCH_BIT_LENGTH = 24;


protected:
  RCSwitch switchSender;

  /*
   * The configuration method is overridden by the specific API servers such as this SwitchServer
   */
  virtual void beginCfg() override;

  /*
   * The methods below contain the library-specific request handling. They call the corresponding
   * core-methods which are library independent.
   */
  void handleSwitchOn();
  void handleSwitchOff();
  void handleUpdateSwitchData();

private:
  bool getSwitchDataParams(SwitchDataParams &switchParams);
  CoreHandlerResponse handleSwitchOnOffCore(bool switchItOn);
};

#endif // MSZ_SWITCHSERVER_H