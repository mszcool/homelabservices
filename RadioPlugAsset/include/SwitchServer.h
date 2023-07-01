#ifndef MSZ_SWITCHSERVER_H
#define MSZ_SWITCHSERVER_H

#include <Arduino.h>

/// @brief Response struct for the core handler methods.
/// @details This struct encapsulates the responses the returned by the core methods for the library specific methods.
struct CoreHandlerResponse {
  int statusCode;
  String contentType;
  String returnContent;
};

/// @brief Defines the parameters for the Switch
/// @details Defines a unique ID for the switch such that the config can be updated, a name, and the command. If isTriState is true, the command is a Tristate while if false, it is a decimal.
struct SwitchDataParams {
  bool isTriState;
  int switchId;
  String switchName;
  String switchCommand;
};

/// @brief Defines the parameters for the metadata
/// @details Defines a token used for securing content, sensor name, and sensor location.
struct SwitchMetadataParams {
  String token;
  String sensorName;
  String sensorLocation;
};

/// @brief Defines a context object for a single request. Needed for some libraries, but not all.
class MszSwitchWebApiRequestContext {

};

/// @class MszSwitchWebApi
/// @brief Switch Web Server class handling on/off requests.
/// @details This class handles the web server which is used to turn on and off radio switches.
class MszSwitchWebApi {
public:
  MszSwitchWebApi(int port);
  void begin();
  void loop();

protected:
  int serverPort;

  /*
   * The methods below contain the library-specific request handling. They call the corresponding
   * core-methods which are library independent.
   */
    void handleSwitchOn(MszSwitchWebApiRequestContext *context);
    void handleSwitchOff(MszSwitchWebApiRequestContext *context);
    void handleUpdateSwitchData(MszSwitchWebApiRequestContext *context);
    void handleUpdateMetadata(MszSwitchWebApiRequestContext *context);

  /*
   * The methods below contain the core logic. They are called by the handlers above.
   * These methods are library-independent and can be used for different platforms.
   */
  CoreHandlerResponse handleSwitchOnCore(String switchName);
  CoreHandlerResponse handleSwitchOffCore(String switchName);
  CoreHandlerResponse handleUpdateSwitchDataCore();
  CoreHandlerResponse handleUpdateMetadataCore();

  /*
   * These are the methods that need to be provided by each, library specific implementation.
   */
  
  /// @brief Defines a template for a context parameter specific to the library.
  virtual void beginServe() = 0;
  virtual void registerEndpoint(String endPoint, std::function<void(MszSwitchWebApiRequestContext*)> handler) = 0;
  virtual String getSwitchNameParameter(MszSwitchWebApiRequestContext *context) = 0;
  virtual SwitchDataParams getSwitchDataParameters(MszSwitchWebApiRequestContext *context) = 0;
  virtual SwitchMetadataParams getSwitchMetadataParameters(MszSwitchWebApiRequestContext *context) = 0;
  virtual void sendResponseData(MszSwitchWebApiRequestContext *context, CoreHandlerResponse responseData) = 0;
};

#endif //MSZ_SWITCHSERVER_H