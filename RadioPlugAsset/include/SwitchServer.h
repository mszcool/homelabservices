#ifndef MSZ_SWITCHSERVER_H
#define MSZ_SWITCHSERVER_H

#include <Arduino.h>
#include <RCSwitch.h>
#include "SwitchData.h"
#include "SecretHandler.h"
#include "SwitchRepository.h"

#define HTTP_OK_CODE 200
#define HTTP_BAD_REQUEST_CODE 400
#define HTTP_UNAUTHORIZED_CODE 401
#define HTTP_NOT_FOUND_CODE 404
#define HTTP_INTERNAL_SERVER_ERROR_CODE 500
#define HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN "text/plain"
#define HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON "application/json"

/// @brief Response struct for the core handler methods.
/// @details This struct encapsulates the responses the returned by the core methods for the library specific methods.
struct CoreHandlerResponse
{
  int statusCode;
  String contentType;
  String returnContent;
};

/// @class MszSwitchWebApi
/// @brief Switch Web Server class handling on/off requests.
/// @details This class handles the web server which is used to turn on and off radio switches.
class MszSwitchWebApi
{
public:
  MszSwitchWebApi(int port);
  void begin(MszSecretHandler *secretHandler);
  void loop();

  static constexpr const char *API_ENDPOINT_INFO = "/info";
  static constexpr const char *API_ENDPOINT_ON = "/switchon";
  static constexpr const char *API_ENDPOINT_OFF = "/switchoff";
  static constexpr const char *API_ENDPOINT_UPDATESWITCHDATA = "/updateswitchdata";
  static constexpr const char *API_ENDPOINT_UPDATEINFO = "/updatemetadata";

  static constexpr const char *API_PARAM_SWITCHID = "switchid";
  static constexpr const char *API_PARAM_SWITCHNAME = "switchname";
  static constexpr const char *API_PARAM_SWITCHCOMMAND = "switchcommand";
  static constexpr const char *API_PARAM_ISTRISTATE = "switchistristate";

  static constexpr const char *HEADER_AUTHORIZATION = "Authorization";
  static constexpr const char *PARAM_SENSOR_NAME = "name";
  static constexpr const char *PARAM_SENSOR_LOCATION = "location";
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
  bool logLoopDone = false;
  int serverPort;

  RCSwitch switchSender;

  // Passed in as a pointer as created outside of the scope of an instance of this class.
  MszSecretHandler *secretHandler;

  /*
   * The methods below contain the library-specific request handling. They call the corresponding
   * core-methods which are library independent.
   */
  bool authorize();
  void handleGetInfo();
  void handleSwitchOn();
  void handleSwitchOff();
  void handleUpdateSwitchData();
  void handleUpdateMetadata();

  /*
   * Authorization releated functions
   */
  void performAuthorizedAction(std::function<CoreHandlerResponse()> action);
  bool validateAuthorizationToken(int timestamp, String token, String signature);

  /*
   * Parameter-retrieval related functions (where needed).
   */
  bool getSwitchDataParams(SwitchDataParams& switchParams);
  bool getMetadataParams(SwitchMetadataParams& metadataParams);

  /*
   * The methods below contain the core logic. They are called by the handlers above.
   * These methods are library-independent and can be used for different platforms.
   */
  CoreHandlerResponse handleGetInfoCore();
  CoreHandlerResponse handleSwitchOnCore();
  CoreHandlerResponse handleSwitchOffCore();
  CoreHandlerResponse handleUpdateSwitchDataCore();
  CoreHandlerResponse handleUpdateMetadataCore();

  /*
   * These are the methods that need to be provided by each, library specific implementation.
   */
  virtual void beginServe() = 0;
  virtual void handleClient() = 0;
  virtual void registerEndpoint(String endPoint, std::function<void()> handler) = 0;
  virtual String getQueryStringParam(String paramName) = 0;
  virtual String getHttpHeader(String headerName) = 0;
  virtual void sendResponseData(CoreHandlerResponse responseData) = 0;
};

#endif // MSZ_SWITCHSERVER_H