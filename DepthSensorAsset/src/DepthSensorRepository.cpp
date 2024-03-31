#include <Arduino.h>
#include <functional>
#include <SPIFFS.h>
#include "DepthSensorEntities.h"
#include "DepthSensorRepository.h"

MszDepthSensorRepository::MszDepthSensorRepository() : AssetBaseRepository()
{
    // In addition to the base class setup of the SPIFFS file systen, here we are initializing the
    // in-memory state of the depth sensor.
    purgeAllMeasurements();
}

DepthSensorConfig MszDepthSensorRepository::loadDepthSensorConfig()
{
    Serial.println("DepthSensorRepository::loadDepthSensorConfig - enter");

    DepthSensorConfig depthSensorConfig;
    File file = SPIFFS.open(DEPTH_SENSOR_CONFIG_FILENAME, "r");
    if (file)
    {
        file.readBytes((char *)&depthSensorConfig, sizeof(depthSensorConfig));
        file.close();
    }
    else
    {
        Serial.println("DepthSensorRepository::loadDepthSensorConfig - failed to open file");
        depthSensorConfig.measureIntervalInSeconds = DEFAULT_MEASURE_INTERVAL_IN_SECONDS;
        depthSensorConfig.measurementsToKeepUntilPurge = MAX_MEASUREMENTS_TO_KEEP_UNTIL_PURGE;
    }

    Serial.println("DepthSensorRepository::loadDepthSensorConfig - exit");
    return depthSensorConfig;
}

bool MszDepthSensorRepository::saveDepthSensorConfig(DepthSensorConfig depthSensorConfig)
{
    Serial.println("DepthSensorRepository::saveDepthSensorConfig - enter");

    Serial.println("DepthSensorRepository::saveDepthSensorConfig - measureIntervalInSeconds = " + String(depthSensorConfig.measureIntervalInSeconds));
    Serial.println("DepthSensorRepository::saveDepthSensorConfig - measurementsToKeepUntilPurge = " + String(depthSensorConfig.measurementsToKeepUntilPurge));

    bool succeeded = false;
    File file = SPIFFS.open(DEPTH_SENSOR_CONFIG_FILENAME, "w");
    if (file)
    {
        file.write((const uint8_t *)&depthSensorConfig, sizeof(depthSensorConfig));
        file.close();
        succeeded = true;
    }
    else
    {
        Serial.println("DepthSensorRepository::saveDepthSensorConfig - failed to open file");
    }

    Serial.println("DepthSensorRepository::saveDepthSensorConfig - exit");
    return succeeded;
}

DepthSensorState MszDepthSensorRepository::loadDepthSensorState()
{
    Serial.println("DepthSensorRepository::loadDepthSensorState - enter and exit");
    return this->inMemoryState;
}

bool MszDepthSensorRepository::addMeasurement(DepthSensorMeasurement measurement)
{
    Serial.println("DepthSensorRepository::addOrUpdateMeasurement - enter");

    bool succeeded = false;
    for (int i = 0; i < MAX_MEASUREMENTS_TO_KEEP_UNTIL_PURGE; i++)
    {
        if (inMemoryState.measurements[i].measurementTime == -1)
        {
            inMemoryState.measurements[i].measurementInCm = measurement.measurementInCm;
            inMemoryState.measurements[i].measurementTime = measurement.measurementTime;
            inMemoryState.measurements[i].hasBeenRetrieved = false;
            succeeded = true;
            break;
        }
    }

    Serial.println("DepthSensorRepository::addOrUpdateMeasurement - exit");
    return succeeded;
}

bool MszDepthSensorRepository::setMeasurementRetrieved(DepthSensorMeasurement measurement)
{
    Serial.println("DepthSensorRepository::addOrUpdateMeasurement - enter");

    bool succeeded = false;
    for (int i = 0; i < MAX_MEASUREMENTS_TO_KEEP_UNTIL_PURGE; i++)
    {
        if (inMemoryState.measurements[i].measurementTime == measurement.measurementTime)
        {
            inMemoryState.measurements[i].hasBeenRetrieved = measurement.hasBeenRetrieved;
            succeeded = true;
            break;
        }
    }

    Serial.println("DepthSensorRepository::addOrUpdateMeasurement - exit");
    return succeeded;
}

bool MszDepthSensorRepository::purgeMeasurements()
{
    this->purgeAllMeasurements();
    return true;
}

void MszDepthSensorRepository::purgeSingleMeasurement(int index)
{
    inMemoryState.measurements[index].hasBeenRetrieved = false;
    inMemoryState.measurements[index].measurementInCm = -1;
    inMemoryState.measurements[index].measurementTime = -1;
}

void MszDepthSensorRepository::purgeAllMeasurements()
{
    for(int i=0; i < MAX_MEASUREMENTS_TO_KEEP_UNTIL_PURGE; i++)
    {
        this->purgeSingleMeasurement(i);
    }
}