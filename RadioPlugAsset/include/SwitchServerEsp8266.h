#if defined(ESP8266)

#ifndef SwitchServerEsp8266_h
#define SwitchServerEsp8266_h

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
  virtual void handleClient() override;
  virtual void registerEndpoint(String endPoint, std::function<void()> handler) override;
  virtual String getTokenFromAuthorizationHeader() override;
  virtual int getTimestampFromAuthorizationHeader() override;
  virtual String getSignatureFromAuthorizationHeader() override;
  virtual String getSwitchNameParameter() override;
  virtual SwitchDataParams getSwitchDataParameters() override;
  virtual SwitchMetadataParams getSwitchMetadataParameters() override;
  virtual void sendResponseData(CoreHandlerResponse responseData) override;
};

#endif // SwitchServerEsp8266_h

#endif // ESP8266