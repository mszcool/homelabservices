#ifndef DEPTHSENSORAPI
#define DEPTHSENSORAPI

#include <Arduino.h>
#include <WebServer.h>
#include <AssetApiBase.h>
#include <SecretHandler.h>
#include <DepthSensorEntities.h>
#include <DepthSensorRepository.h>

/// @brief API for the Depth Sensor
/// @details This class is responsible for handling the API calls for the Depth Sensor.
class MszDepthSensorApi
: public MszAssetApiBase
{
public:
    MszDepthSensorApi(MszDepthSensorRepository *depthRepository);
    MszDepthSensorApi(MszDepthSensorRepository *depthRepository, short secretId, int serverPort);

    static const int HTTP_AUTH_SECRET_ID = 0;

    static constexpr const char *API_ENDPOINT_DEPTH_SENSOR_CONFIG = "/config";
    static constexpr const char *API_ENDPOINT_DEPTH_SENSOR_GETMEASUREMENTS = "/measurements";

    static constexpr const char *API_PARAM_CONFIG_MEASUREMENT_INTERVAL = "measurementintervalseconds";
    static constexpr const char *API_PARAM_CONFIG_MEASUREMENTS_TOKEEP = "measurementstokeep";

protected:
    WebServer server;
    MszDepthSensorRepository *depthSensorRepository = NULL;

    /*
     * The configuration method is overridden by the specific API servers such as this DepthSensorApi
     */
    virtual void beginCfg() override;

    /*
     * The methods below contain the library-specific request handling. They call the corresponding
     * core-methods which are library independent.
     */
    void handleGetDepthSensorConfig();
    void handleUpdateDepthSensorConfig();
    void handleGetDepthSensorMeasurements();
    void handlePurgeDepthSensorMeasurements();

    /*
     * Overrides for the actual web server handling methods 
     */
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

#endif // DEPTHSENSORAPI