#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include <Arduino.h>
#include <ArduinoJson.h>

#include "DepthSensorEntities.h"
#include "DepthSensorRepository.h"
#include "DepthSensorWebApi.h"

MszDepthSensorApi::MszDepthSensorApi(MszDepthSensorRepository *depthRepository)
    : MszAssetApiBase()
{
    this->depthSensorRepository = depthRepository;
}

MszDepthSensorApi::MszDepthSensorApi(MszDepthSensorRepository *depthRepository, short secretId, int serverPort)
    : MszAssetApiBase(secretId, serverPort), server(serverPort)
{
    this->depthSensorRepository = depthRepository;
}

void MszDepthSensorApi::beginCfg()
{
    Serial.println("MszDepthSensorApi::beginCfg() - enter");

    Serial.println("MszDepthSensorApi::beginCfg() - Configuring Depth Sensor API secret handler");
    this->registerGetEndpoint(API_ENDPOINT_DEPTH_SENSOR_CONFIG, std::bind(&MszDepthSensorApi::handleGetDepthSensorConfig, this));
    this->registerPutEndpoint(API_ENDPOINT_DEPTH_SENSOR_CONFIG, std::bind(&MszDepthSensorApi::handleUpdateDepthSensorConfig, this));
    this->registerGetEndpoint(API_ENDPOINT_DEPTH_SENSOR_GETMEASUREMENTS, std::bind(&MszDepthSensorApi::handleGetDepthSensorMeasurements, this));
    this->registerDeleteEndpoint(API_ENDPOINT_DEPTH_SENSOR_GETMEASUREMENTS, std::bind(&MszDepthSensorApi::handlePurgeDepthSensorMeasurements, this));
    Serial.println("MszDepthSensorApi::beginCfg() - Depth Sensor API endpoints configured!");

    Serial.println("MszDepthSensorApi::beginCfg() - exit");
}

void MszDepthSensorApi::beginServe()
{
    this->server.begin();
}

void MszDepthSensorApi::handleClient()
{
    this->server.handleClient();
}

void MszDepthSensorApi::registerGetEndpoint(String endpoint, std::function<void()> handler)
{
    server.on(endpoint.c_str(), HTTP_GET, handler);
}

void MszDepthSensorApi::registerPostEndpoint(String endpoint, std::function<void()> handler)
{
    server.on(endpoint.c_str(), HTTP_POST, handler);
}

void MszDepthSensorApi::registerPutEndpoint(String endpoint, std::function<void()> handler)
{
    server.on(endpoint.c_str(), HTTP_PUT, handler);
}

void MszDepthSensorApi::registerDeleteEndpoint(String endpoint, std::function<void()> handler)
{
    server.on(endpoint.c_str(), HTTP_DELETE, handler);
}

String MszDepthSensorApi::getQueryStringParam(String paramName)
{
    return server.arg(paramName.c_str());
}

String MszDepthSensorApi::getHttpHeader(String headerName)
{
    return server.header(headerName.c_str());
}

void MszDepthSensorApi::sendResponseData(CoreHandlerResponse response)
{
    Serial.println("Sending response data - enter.");
    server.send(response.statusCode, response.contentType, response.returnContent);
    Serial.println("Sending response data - exit.");
}

void MszDepthSensorApi::handleGetDepthSensorConfig()
{
    Serial.println("Depth Sensor API handleGetDepthSensorConfig - enter");
    performAuthorizedAction([&]()
    {
        Serial.println("Depth Sensor API handleGetDepthSensorConfig - authorized, performing action");
        CoreHandlerResponse response;

        DepthSensorConfig config = this->depthSensorRepository->loadDepthSensorConfig();
        response.statusCode = HTTP_OK_CODE;
        response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;
        
        // Create the JSON content for the depth sensor configuration using serializeJsonPretty
        JsonDocument responseDoc;
        responseDoc["isDefault"] = config.isDefault;
        responseDoc["measurementIntervalSeconds"] = config.measureIntervalInSeconds;
        responseDoc["measurementsToKeep"] = config.measurementsToKeepUntilPurge;
        serializeJsonPretty(responseDoc, response.returnContent);

        Serial.println("Depth Sensor API handleGetDepthSensorConfig - authorized action exit");
        return response; 
    });
    Serial.println("Depth Sensor API handleGetDepthSensorConfig - exit");
}

void MszDepthSensorApi::handleUpdateDepthSensorConfig()
{
    Serial.println("Depth Sensor API handleUpdateDepthSensorConfig - enter");
    performAuthorizedAction([&]()
    {
        Serial.println("Depth Sensor API handleUpdateDepthSensorConfig - authorized, performing action");
        DepthSensorConfig config;
        CoreHandlerResponse response;

        bool validationSucceeded = true;

        // First, get the configuration parameters from the request.
        String measureIntervalString = this->getQueryStringParam(API_PARAM_CONFIG_MEASUREMENT_INTERVAL);
        String measurementsKeepString = this->getQueryStringParam(API_PARAM_CONFIG_MEASUREMENTS_TOKEEP);

        // Validate if all required parameters are present.
        if (measureIntervalString == nullptr || measureIntervalString == "" || measurementsKeepString == nullptr || measurementsKeepString == "")
        {
            validationSucceeded = false;
        }

        // Validate if both parameters are integers.
        std::istringstream intValidator(measureIntervalString.c_str());
        intValidator >> std::noskipws >> config.measureIntervalInSeconds;
        if (intValidator.fail())
        {
            validationSucceeded = false;
        }
        intValidator.clear();
        intValidator.str(measurementsKeepString.c_str());
        intValidator >> std::noskipws >> config.measurementsToKeepUntilPurge;
        if (intValidator.fail())
        {
            validationSucceeded = false;
        }

        // If any of the validations failed above, return a bad request response.
        if (!validationSucceeded)
        {
            Serial.println("Depth Sensor API handleUpdateDepthSensorConfig - config data invalid");

            response.statusCode = HTTP_BAD_REQUEST_CODE;
            response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;
            response.returnContent = this->getErrorJsonDocument(
                HTTP_BAD_REQUEST_CODE,
                "Invalid Depth Sensor Config Data!",
                "You did not provide valid depth sensor config data for updating the depth sensor!");

            Serial.println("Depth Sensor API handleUpdateDepthSensorConfig - exit");
            return response;
        }

        // If all parameters are validated, execute the core logic.
        config.isDefault = false;
        bool succeeded = this->depthSensorRepository->saveDepthSensorConfig(config);

        response.statusCode = (succeeded ? HTTP_OK_CODE : HTTP_INTERNAL_SERVER_ERROR_CODE);
        response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;

        JsonDocument respDoc;
        respDoc["isDefault"] = config.isDefault;
        respDoc["measurementIntervalSeconds"] = config.measureIntervalInSeconds;
        respDoc["measurementsToKeep"] = config.measurementsToKeepUntilPurge;
        respDoc["configStatus"] = (succeeded ? "CONFIG_UPDATED" : "CONFIG_UPDATE_FAILED");
        serializeJsonPretty(respDoc, response.returnContent);

        Serial.println("Depth Sensor API handleUpdateDepthSensorConfig - exit");
        return response;
    });
    Serial.println("Depth Sensor API handleUpdateDepthSensorConfig - exit");
}

void MszDepthSensorApi::handleGetDepthSensorMeasurements()
{
    Serial.println("Depth Sensor API handleGetDepthSensorMeasurements - enter");
    performAuthorizedAction([&]()
    {
        Serial.println("Depth Sensor API handleGetDepthSensorMeasurements - authorized, performing action");
        CoreHandlerResponse response;

        DepthSensorState state;
        state = this->depthSensorRepository->loadDepthSensorState();
        response.statusCode = HTTP_OK_CODE;
        response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;

        // Create the JSON content for the depth sensor state using serializeJsonPretty
        JsonDocument responseDoc;
        JsonArray measurementsArray = responseDoc["measurements"].to<JsonArray>();
        for (int i = 0; i < state.measurementCount; i++)
        {
            JsonObject measurement = measurementsArray.add<JsonObject>();
            measurement["measureTime"] = state.measurements[i].measurementTime;
            measurement["centimeters"] = state.measurements[i].measurementInCm;
            measurement["retrievedBefore"] = state.measurements[i].hasBeenRetrieved;
            this->depthSensorRepository->setMeasurementRetrieved(i);
        }
        serializeJsonPretty(responseDoc, response.returnContent);

        Serial.println("Depth Sensor API handleGetDepthSensorMeasurements - authorized action exit");
        return response;
    });
    Serial.println("Depth Sensor API handleGetDepthSensorMeasurements - exit");
}

void MszDepthSensorApi::handlePurgeDepthSensorMeasurements()
{
    Serial.println("Depth Sensor API handlePurgeDepthSensorMeasurements - enter");
    performAuthorizedAction([&]()
    {
        Serial.println("Depth Sensor API handlePurgeDepthSensorMeasurements - authorized, performing action");
        CoreHandlerResponse response;

        bool succeeded = this->depthSensorRepository->purgeMeasurements();
        response.statusCode = (succeeded ? HTTP_OK_CODE : HTTP_INTERNAL_SERVER_ERROR_CODE);
        response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;

        JsonDocument respDoc;
        respDoc["purgeStatus"] = (succeeded ? "PURGE_SUCCESS" : "PURGE_FAILED");
        serializeJsonPretty(respDoc, response.returnContent);

        Serial.println("Depth Sensor API handlePurgeDepthSensorMeasurements - exit");
        return response;
    });
    Serial.println("Depth Sensor API handlePurgeDepthSensorMeasurements - exit");
}