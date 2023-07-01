#if defined(ESP8266)

#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include "SwitchServer.h"

/// @brief ESP8266 implementation of the MszSwitchWebApi class.
/// @details This class implements the MszSwitchWebApi class for the ESP8266 platform.
class MszSwitchApiEsp8266 : public MszSwitchWebApi {
public:
  MszSwitchApiEsp8266(int port);

protected:
  ESP8266WebServer server;
  
  virtual void beginServe() override;
  virtual void registerEndpoint(String endPoint, std::function<void(MszSwitchWebApiRequestContext*)> handler) override;
  virtual String getSwitchNameParameter(MszSwitchWebApiRequestContext *context) override;
  virtual SwitchDataParams getSwitchDataParameters(MszSwitchWebApiRequestContext *context) override;
  virtual SwitchMetadataParams getSwitchMetadataParameters(MszSwitchWebApiRequestContext *context) override;
  virtual void sendResponseData(MszSwitchWebApiRequestContext *context, CoreHandlerResponse responseData) override;
};

#endif