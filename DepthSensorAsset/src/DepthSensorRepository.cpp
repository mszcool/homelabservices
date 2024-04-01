#include <Arduino.h>
#include <functional>
#include <SPIFFS.h>
#include <TimeLib.h>
#include "DepthSensorEntities.h"
#include "DepthSensorRepository.h"

DepthSensorState MszDepthSensorRepository::inMemoryState;

MszDepthSensorRepository::MszDepthSensorRepository() : AssetBaseRepository()
{
    // In addition to the base class setup of the SPIFFS file system, here we are initializing the
    // in-memory state of the depth sensor.
    purgeAllMeasurements();

    // Set the default configuration values.
    inMemoryState.currentConfig.isDefault = true;
    inMemoryState.currentConfig.measureIntervalInSeconds = DEFAULT_MEASURE_INTERVAL_IN_SECONDS;
    inMemoryState.currentConfig.measurementsToKeepUntilPurge = MAX_MEASUREMENTS_TO_KEEP_UNTIL_PURGE;
    inMemoryState.lastConfigTimeRead = 0;
    inMemoryState.lastConfigTimeWrite = 0;
}

DepthSensorConfig MszDepthSensorRepository::loadDepthSensorConfig()
{
    Serial.println("DepthSensorRepository::loadDepthSensorConfig - enter");

    Serial.println("DepthSensorRepository::loadDepthSensorConfig - lastConfigTimeRead = " + String(inMemoryState.lastConfigTimeRead));
    Serial.println("DepthSensorRepository::loadDepthSensorConfig - lastConfigTimeWrite = " + String(inMemoryState.lastConfigTimeWrite));

    if (inMemoryState.currentConfig.isDefault)
    {
        Serial.println("DepthSensorRepository::loadDepthSensorConfig - default configuration still present, loading configuration from file if needed.");

        // First check, if the configuration read is still the same as the configuration written, and if it has ever been read, before.
        if ((inMemoryState.lastConfigTimeRead != 0) && (inMemoryState.lastConfigTimeRead == inMemoryState.lastConfigTimeWrite))
        {
            Serial.println("DepthSensorRepository::loadDepthSensorConfig - in-memory configuration is still the same as configuration on file.");
            return inMemoryState.currentConfig;
        }

        // The configuration is out of sync, or it has never been read before, hence it is worth checking.
        bool fileExists = SPIFFS.exists(DEPTH_SENSOR_CONFIG_FILENAME);
        if (fileExists)
        {
            DepthSensorConfig readConfigFromFile;
            File file = SPIFFS.open(DEPTH_SENSOR_CONFIG_FILENAME, "r");
            if (file)
            {
                file.readBytes((char *)&readConfigFromFile, sizeof(readConfigFromFile));
                file.close();

                // After successfully reading content from file, updated the in-memory state.
                inMemoryState.currentConfig = readConfigFromFile;
                inMemoryState.lastConfigTimeRead = now();
            }
            else
            {
                Serial.println("DepthSensorRepository::loadDepthSensorConfig - failed to open file");
            }
        }
    }

    Serial.println("DepthSensorRepository::loadDepthSensorConfig - exit");
    return inMemoryState.currentConfig;
}

bool MszDepthSensorRepository::saveDepthSensorConfig(DepthSensorConfig depthSensorConfig)
{
    Serial.println("DepthSensorRepository::saveDepthSensorConfig - enter");

    Serial.println("DepthSensorRepository::saveDepthSensorConfig - measureIntervalInSeconds = " + String(depthSensorConfig.measureIntervalInSeconds));
    Serial.println("DepthSensorRepository::saveDepthSensorConfig - measurementsToKeepUntilPurge = " + String(depthSensorConfig.measurementsToKeepUntilPurge));

    Serial.println("DepthSensorRepository::saveDepthSensorConfig - Saving means the configuration is not considered default, anymore!");
    inMemoryState.currentConfig.isDefault = false;

    Serial.println("DepthSensorRepository::saveDepthSensorConfig - lastConfigTimeRead = " + String(inMemoryState.lastConfigTimeRead));
    Serial.println("DepthSensorRepository::saveDepthSensorConfig - lastConfigTimeWrite = " + String(inMemoryState.lastConfigTimeWrite));

    bool succeeded = false;
    File file = SPIFFS.open(DEPTH_SENSOR_CONFIG_FILENAME, "w");
    if (file)
    {
        file.write((const uint8_t *)&depthSensorConfig, sizeof(depthSensorConfig));
        file.close();

        // Updating time when the file was written last time.
        inMemoryState.lastConfigTimeWrite = now();
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
    return inMemoryState;
}

bool MszDepthSensorRepository::addMeasurement(DepthSensorMeasurement measurement)
{
    Serial.println("DepthSensorRepository::addOrUpdateMeasurement - enter");

    bool succeeded = false;

    if (inMemoryState.measurementCount >= MAX_MEASUREMENTS_TO_KEEP_UNTIL_PURGE)
    {
        Serial.println("DepthSensorRepository::addOrUpdateMeasurement - max measurements reached, purging");
        this->purgeAllMeasurements();
        inMemoryState.measurementCount = 0;
    }

    // Now store the values in the new target measurement.
    inMemoryState.measurements[inMemoryState.measurementCount].measurementInCm = measurement.measurementInCm;
    inMemoryState.measurements[inMemoryState.measurementCount].measurementTime = measurement.measurementTime;
    inMemoryState.measurements[inMemoryState.measurementCount].hasBeenRetrieved = false;
    inMemoryState.measurementCount++;

    Serial.println("DepthSensorRepository::addOrUpdateMeasurement - exit");
    return succeeded;
}

bool MszDepthSensorRepository::setMeasurementRetrieved(int index)
{
    Serial.println("DepthSensorRepository::setMeasurementRetrieved - enter");

    bool succeeded = false;
    if (index >= 0 && index < MAX_MEASUREMENTS_TO_KEEP_UNTIL_PURGE)
    {
        inMemoryState.measurements[index].hasBeenRetrieved = true;
        succeeded = true;
    }

    Serial.println("DepthSensorRepository::setMeasurementRetrieved - exit");
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
    inMemoryState.measurementCount = 0;
    for(int i=0; i < MAX_MEASUREMENTS_TO_KEEP_UNTIL_PURGE; i++)
    {
        this->purgeSingleMeasurement(i);
    }
}