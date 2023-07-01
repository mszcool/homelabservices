#if defined(ESP32)

#ifndef SwitchServerEsp32_h
#define SwitchServerEsp32_h

#include "SwitchServer.h"
#include <ESPAsyncWebServer.h>

/// @brief Defines a context object for a single request. Needed for some libraries, but not all.
class MszSwitchWebApiEsp32RequestContext : public MszSwitchWebApiRequestContext {
public:
  AsyncWebServerRequest *request;
};

/// @brief ESP32 implementation of the MszSwitchWebApi class.
/// @details This class implements the MszSwitchWebApi class for the ESP32 platform.
class MszSwitchApiEsp32 : public MszSwitchWebApi {
public :
  MszSwitchApiEsp32(int port);

protected:
  AsyncWebServer server;
  
  virtual void beginServe() override;
  virtual void registerEndpoint(String endPoint, std::function<void(MszSwitchWebApiRequestContext*)> handler) override;
  virtual String getSwitchNameParameter(MszSwitchWebApiRequestContext *context) override;
  virtual SwitchDataParams getSwitchDataParameters(MszSwitchWebApiRequestContext *context) override;
  virtual SwitchMetadataParams getSwitchMetadataParameters(MszSwitchWebApiRequestContext *context) override;
  virtual void sendResponseData(MszSwitchWebApiRequestContext *context, CoreHandlerResponse responseData) override;
};

#endif // SwitchServerEsp32_h

#endif // ESP32