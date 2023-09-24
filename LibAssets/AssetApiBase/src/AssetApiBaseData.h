#ifndef MSZ_ASSETAPIBASEDATA_H
#define MSZ_ASSETAPIBASEDATA_H

#include <Arduino.h>

#define MAX_SENSOR_NAME_LENGTH 32
#define MAX_SENSOR_LOCATION_LENGTH 64

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
};

#endif // MSZ_ASSETAPIBASEDATA_H