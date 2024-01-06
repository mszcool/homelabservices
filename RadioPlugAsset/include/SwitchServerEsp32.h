#if defined(ESP32)

#ifndef SwitchServerEsp32_h
#define SwitchServerEsp32_h

#include "SwitchServer.h"
#include <WebServer.h>

/// @brief ESP32 implementation of the MszSwitchWebApi class.
/// @details This class implements the MszSwitchWebApi class for the ESP32 platform.
class MszSwitchApiEsp32 : public MszSwitchWebApi {
public :
  MszSwitchApiEsp32(short secretId, int serverPort);

protected:
  WebServer server;
  
  virtual void beginServe() override;
  virtual void handleClient() override;
  virtual void registerGetEndpoint(String endPoint, std::function<void()> handler) override;
  virtual void registerPostEndpoint(String endPoint, std::function<void()> handler) override;
  virtual void registerPutEndpoint(String endPoint, std::function<void()> handler) override;
  virtual void registerDeleteEndpoint(String endPoint, std::function<void()> handler) override;
  virtual String getQueryStringParam(String paramName) override;
  virtual String getHttpHeader(String headerName) override;
  virtual void sendResponseData(CoreHandlerResponse responseData) override;
};

#endif // SwitchServerEsp32_h

#endif // ESP32