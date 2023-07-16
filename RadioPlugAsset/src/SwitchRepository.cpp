#include <Arduino.h>
#include <functional>
#include "SwitchRepository.h"

#ifdef ESP32

#include <SPIFFS.h>

MszSwitchRepository::MszSwitchRepository()
{
    Serial.println("SwitchRepository::SwitchRepository - enter");

    if (!SPIFFS.begin())
    {
        Serial.println("Failed to mount file system, formatting...");
        SPIFFS.format();
        if(!SPIFFS.begin())
        {
            Serial.println("Failed to mount file system, aborting...");
            return;
        }
    }

    Serial.println("SwitchRepository::SwitchRepository - exit");
}

SwitchMetadataParams MszSwitchRepository::loadMetadata()
{
    Serial.println("SwitchRepository::loadMetadata - enter");

    SwitchMetadataParams metadata;
    File file = SPIFFS.open(SWITCH_METADATA_FILENAME, "r");
    if (file)
    {
        file.readBytes((char *)&metadata, sizeof(metadata));
        file.close();
    }
    else
    {
        Serial.println("SwitchRepository::loadMetadata - failed to open file - returning defaults");
        metadata.sensorName[0] = '\0';
        metadata.sensorLocation[0] = '\0';
    }

    Serial.println("SwitchRepository::loadMetadata - sensorName = " + String(metadata.sensorName));
    Serial.println("SwitchRepository::loadMetadata - sensorLocation = " + String(metadata.sensorLocation));
    Serial.println("SwitchRepository::loadMetadata - exit");
    return metadata;
}

bool MszSwitchRepository::saveMetadata(SwitchMetadataParams metadata)
{
    Serial.println("SwitchRepository::saveMetadata - enter");

    bool succeeded = false;
    File file = SPIFFS.open(SWITCH_METADATA_FILENAME, "w");
    if (file)
    {
        file.write((const uint8_t *)&metadata, sizeof(metadata));
        file.close();
        succeeded = true;
    }
    else
    {
        Serial.println("SwitchRepository::saveMetadata - failed to open file");
    }

    Serial.println("SwitchRepository::saveMetadata - exit");
    return succeeded;
}

SwitchDataParams MszSwitchRepository::loadSwitchData(String switchName)
{
    Serial.println("SwitchRepository::loadSwitchData - enter");

    SwitchDataParams switchData;
    String fileName = String(SWITCH_FILENAME_PREFIX) + switchName;
    File file = SPIFFS.open(fileName.c_str(), "r");
    if (file)
    {
        file.readBytes((char *)&switchData, sizeof(switchData));
        file.close();
    }
    else
    {
        Serial.println("SwitchRepository::loadSwitchData - failed to open file");
        switchData.isTriState = false;
        switchData.switchProtocol = 0;
        switchData.switchName[0] = '\0';
        switchData.switchOnCommand[0] = '\0';
        switchData.switchOffCommand[0] = '\0';
    }

    Serial.println("SwitchRepository::loadSwitchData - switchName = " + String(switchData.switchName));
    Serial.println("SwitchRepository::loadSwitchData - switchProtocol = " + String(switchData.switchProtocol));
    Serial.println("SwitchRepository::loadSwitchData - switchOnCommand = " + String(switchData.switchOnCommand));
    Serial.println("SwitchRepository::loadSwitchData - switchOffCommand = " + String(switchData.switchOffCommand));
    Serial.println("SwitchRepository::loadSwitchData - isTriState = " + String(switchData.isTriState));
    Serial.println("SwitchRepository::loadSwitchData - exit");
    return switchData;
}

bool MszSwitchRepository::saveSwitchData(String switchName, SwitchDataParams switchDataParams)
{
    Serial.println("SwitchRepository::saveSwitchData - enter");

    bool succeeded = false;
    String fileName = String(SWITCH_FILENAME_PREFIX) + switchName;
    File file = SPIFFS.open(fileName.c_str(), "w");
    if (file)
    {
        file.write((const uint8_t *)&switchDataParams, sizeof(switchDataParams));
        file.close();
        succeeded = true;
    }
    else
    {
        Serial.println("SwitchRepository::saveSwitchData - failed to open file");
    }

    Serial.println("SwitchRepository::saveSwitchData - exit");
    return succeeded;
}

#endif