#include <Arduino.h>
#include <functional>
#include "SwitchRepository.h"

#ifdef ESP32

#include <LittleFS.h>

MszSwitchRepository::MszSwitchRepository()
{
    Serial.println("SwitchRepository::SwitchRepository - enter");
    if (!LittleFS.begin())
    {
        Serial.println("An error occurred while mounting LittleFS");
    }
    Serial.println("SwitchRepository::SwitchRepository - exit");
}

SwitchMetadataParams MszSwitchRepository::loadMetadata()
{
    Serial.println("SwitchRepository::loadMetadata - enter");

    SwitchMetadataParams metadata;
    File file = LittleFS.open(SWITCH_METADATA_FILENAME, "r");
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

    Serial.println("SwitchRepository::loadMetadata - exit");
    return metadata;
}

bool MszSwitchRepository::saveMetadata(SwitchMetadataParams metadata)
{
    Serial.println("SwitchRepository::saveMetadata - enter");

    bool succeeded = false;
    File file = LittleFS.open(SWITCH_METADATA_FILENAME, "w");
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
    File file = LittleFS.open(fileName.c_str(), "r");
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

    Serial.println("SwitchRepository::loadSwitchData - exit");
    return switchData;
}

bool MszSwitchRepository::saveSwitchData(String switchName, SwitchDataParams switchDataParams)
{
    Serial.println("SwitchRepository::saveSwitchData - enter");

    bool succeeded = false;
    String fileName = String(SWITCH_FILENAME_PREFIX) + switchName;
    File file = LittleFS.open(fileName.c_str(), "w");
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