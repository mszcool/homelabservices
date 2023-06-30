#ifndef MSZ_SWITCHSERVER_H
#define MSZ_SWITCHSERVER_H

#include <Arduino.h>

#if defined(ESP8266)
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#elif defined(ESP32)
#include <ESPAsyncWebServer.h>
#endif

/// @brief Response struct for the core handler methods.
/// @details This struct encapsulates the responses the returned by the core methods for the library specific methods.
struct CoreHandlerResponse {
  int statusCode;
  String contentType;
  String returnContent;
};

/// @class MszSwitchWebApi
/// @brief Switch Web Server class handling on/off requests.
/// @details This class handles the web server which is used to turn on and off radio switches.
class MszSwitchWebApi {
public:
  MszSwitchWebApi(int port);
  void begin();
  void loop();

private:
#if defined(ESP8266)
  ESP8266WebServer server;
#elif defined(ESP32)
  AsyncWebServer server;
#endif

  /*
   * The methods below contain the library-specific request handling. They call the corresponding
   * core-methods which are library independent.
   */
  void handleSwitchOn();
  void handleSwitchOff();
  void handleUpdateSwitchData();
  void handleUpdateMetadata();

  /*
   * The methods below contain the core logic. They are called by the handlers above.
   * These methods are library-independent and can be used for different platforms.
   */
  CoreHandlerResponse handleSwitchOnCore(String switchName);
  CoreHandlerResponse handleSwitchOffCore(String switchName);
  CoreHandlerResponse handleUpdateSwitchDataCore();
  CoreHandlerResponse handleUpdateMetadataCore();
};

#endif
