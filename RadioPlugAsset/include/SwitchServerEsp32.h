#if defined(ESP32)

#ifndef SwitchServerEsp32_h
#define SwitchServerEsp32_h

#include "SwitchServer.h"
#include <WebServer.h>

/// @brief ESP32 implementation of the MszSwitchWebApi class.
/// @details This class implements the MszSwitchWebApi class for the ESP32 platform.
class MszSwitchApiEsp32 : public MszSwitchWebApi {
public :
  MszSwitchApiEsp32(int port);

protected:
  WebServer server;
  
  virtual void beginServe() override;
  virtual void handleClient() override;
  virtual void registerEndpoint(String endPoint, std::function<void()> handler) override;
  virtual String getTokenAuthorizationHeader() override;
  virtual String getTokenSignatureHeader() override;
  virtual String getSwitchNameParameter() override;
  virtual SwitchDataParams getSwitchDataParameters() override;
  virtual SwitchMetadataParams getSwitchMetadataParameters() override;
  virtual void sendResponseData(CoreHandlerResponse responseData) override;
};

#endif // SwitchServerEsp32_h

#endif // ESP32