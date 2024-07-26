#ifndef MSZ_ASSETAPIBASE
#define MSZ_ASSETAPIBASE

#include <Arduino.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include "SecretHandler.h"
#include "AssetApiBaseData.h"

#define HTTP_OK_CODE 200
#define HTTP_BAD_REQUEST_CODE 400
#define HTTP_UNAUTHORIZED_CODE 401
#define HTTP_NOT_FOUND_CODE 404
#define HTTP_INTERNAL_SERVER_ERROR_CODE 500
#define HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN "text/plain"
#define HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON "application/json"

/// @class MszAssetApiBase
/// @brief Base class for the asset web API based on a simple web server.
/// @details This class provides the base functionality I use across all Web APIs on my assets, especially authorization and token validation.
class MszAssetApiBase
{
public:
    MszAssetApiBase();
    MszAssetApiBase(short secretId, int serverPort);
    virtual ~MszAssetApiBase();

    void begin(MszSecretHandler *secretHandler);
    void loop();

    static constexpr const char *API_ENDPOINT_INFO = "/info";
    static constexpr const char *API_ENDPOINT_UPDATEINFO = "/updateinfo";
    static constexpr const char *API_ENDPOINT_SETTIME = "/settime";

    static constexpr const char *HEADER_AUTHORIZATION = "Authorization";
    static constexpr const char *PARAM_SENSOR_NAME = "name";
    static constexpr const char *PARAM_SENSOR_LOCATION = "location";
    static constexpr const char *PARAM_SENSOR_MQTT_SERVER = "mqttserver";
    static constexpr const char *PARAM_SENSOR_MQTT_USERNAME = "mqttusername";
    static constexpr const char *PARAM_SENSOR_MQTT_PASSWORD = "mqttpassword";

    static constexpr const char* PARAM_HOUR = "hour";
    static constexpr const char* PARAM_MINUTE = "minute";
    static constexpr const char* PARAM_SECOND = "second";
    static constexpr const char* PARAM_DAY = "day";
    static constexpr const char* PARAM_MONTH = "month";
    static constexpr const char* PARAM_YEAR = "year";

    static const int TOKEN_EXPIRATION_SECONDS = 60;

protected:
    // Basic members.
    bool logLoopDone = false;
    int serverPort = 80;
    short secretId = 0;

    // Passed in as a pointer as created outside of the scope of an instance of this class.
    MszSecretHandler *secretHandler;

    // Authorization related methods re-used across all implementations.
    bool authorize();
    void performAuthorizedAction(std::function<CoreHandlerResponse()> action);
    bool validateAuthorizationToken(int timestamp, String token, String signature);
    String getErrorJsonDocument(int errorCode, String errorTitle, String errorMessage);

    /*
     * Web API Handler Methods provided to all derived implementations.
     */
    void handleGetInfo();
    void handleUpdateInfo();
    void handleSetSensorTime();

    /*
     * These are the methods that need to be provided by each, library specific implementation.
     */
    virtual void beginCfg() = 0;        // Called by derived classes to customize endpoint registration and do pre-serve configuration.
    virtual void beginServe() = 0;      // Called by derived classes to launch the web server implementation.
    virtual void handleClient() = 0;
    virtual void registerGetEndpoint(String endPoint, std::function<void()> handler) = 0;
    virtual void registerPostEndpoint(String endPoint, std::function<void()> handler) = 0;
    virtual void registerPutEndpoint(String endPoint, std::function<void()> handler) = 0;
    virtual void registerDeleteEndpoint(String endPoint, std::function<void()> handler) = 0;
    virtual String getQueryStringParam(String paramName) = 0;
    virtual String getHttpHeader(String headerName) = 0;
    virtual void sendResponseData(CoreHandlerResponse responseData) = 0;

private:
    // Private helper methods.
    bool getMetadataParams(AssetMetadataParams &metadataParams);
    String getMetadataJson(String status, AssetMetadataParams &params);
};

#endif // MSZ_ASSETAPIBASE