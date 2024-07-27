#ifndef MSZ_ASSETAPIBASEDATA_H
#define MSZ_ASSETAPIBASEDATA_H

#include <Arduino.h>

#define MAX_SENSOR_NAME_LENGTH 32
#define MAX_SENSOR_LOCATION_LENGTH 64
#define MAX_MQTT_SERVER_NAME 128
#define MAX_MQTT_USERNAME 64
#define MAX_MQTT_PASSWORD 64

/// @brief Response struct for the core handler methods.
/// @details This struct encapsulates the responses the returned by the core methods for the library specific methods.
struct CoreHandlerResponse
{
    int statusCode;
    String contentType;
    String returnContent;
};

/// @brief Defines the parameters for the metadata
/// @details Defines a token used for securing content, sensor name, and sensor location.
struct AssetMetadataParams
{
  char sensorName[MAX_SENSOR_NAME_LENGTH+1];
  char sensorLocation[MAX_SENSOR_LOCATION_LENGTH+1];
  char sensorMqttServer[MAX_MQTT_SERVER_NAME+1];
  int sensorMqttPort = 1883;
  char sensorMqttUsername[MAX_MQTT_USERNAME+1];
  char sensorMqttPassword[MAX_MQTT_PASSWORD+1];
};

/// @brief Base repository for assets
/// @details Defines the base class for a repository implementation
class AssetBaseRepository
{
  public:
    AssetBaseRepository();
    virtual ~AssetBaseRepository();

    static constexpr const char *ASSET_METADATA_FILENAME = "/swm";

    AssetMetadataParams loadMetadata();
    bool saveMetadata(AssetMetadataParams metadata);
};

#endif //MSZ_ASSETAPIBASEDATA_H