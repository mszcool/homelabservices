#ifndef DEPTHSENSORREPOSITORY
#define DEPTHSENSORREPOSITORY

#include <DepthSensorEntities.h>
#include <AssetApiBase.h>

/// @brief Repository for the Depth Sensor
/// @details This class is responsible for loading and saving the Depth Sensor configuration and state and for keeping the measurements.
class MszDepthSensorRepository
    : public AssetBaseRepository
{
private:
    static DepthSensorState inMemoryState;

public:
    MszDepthSensorRepository();

    static constexpr const char *DEPTH_SENSOR_FILENAME_PREFIX = "/depth";
    static constexpr const char *DEPTH_SENSOR_CONFIG_FILENAME = "/sensorConfig";

    DepthSensorConfig loadDepthSensorConfig();
    bool saveDepthSensorConfig(DepthSensorConfig depthSensorConfig);

    DepthSensorState loadDepthSensorState();
    bool addMeasurement(DepthSensorMeasurement measurement);
    bool setMeasurementRetrieved(int index);
    bool purgeMeasurements();

private:
    void purgeSingleMeasurement(int index);
    void purgeAllMeasurements();
};

#endif // DEPTHSENSORREPOSITORY