#include <Arduino.h>
#include <functional>
#include "SwitchRepository.h"

#ifdef ESP32

#include <SPIFFS.h>

MszSwitchRepository::MszSwitchRepository() : AssetBaseRepository()
{
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

    Serial.println("SwitchRepository::saveSwitchData - switchName = " + String(switchName));
    Serial.println("SwitchRepository::saveSwitchData - switchName = " + String(switchDataParams.switchName));
    Serial.println("SwitchRepository::saveSwitchData - switchProtocol = " + String(switchDataParams.switchProtocol));
    Serial.println("SwitchRepository::saveSwitchData - switchOnCommand = " + String(switchDataParams.switchOnCommand));
    Serial.println("SwitchRepository::saveSwitchData - switchOffCommand = " + String(switchDataParams.switchOffCommand));
    Serial.println("SwitchRepository::saveSwitchData - isTriState = " + String(switchDataParams.isTriState));

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