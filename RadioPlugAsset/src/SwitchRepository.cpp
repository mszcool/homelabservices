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
        switchData.pulseLength = 0;
        switchData.repeatTransmit = 0;
    }

    Serial.println("SwitchRepository::loadSwitchData - switchName = " + String(switchData.switchName));
    Serial.println("SwitchRepository::loadSwitchData - switchProtocol = " + String(switchData.switchProtocol));
    Serial.println("SwitchRepository::loadSwitchData - switchOnCommand = " + String(switchData.switchOnCommand));
    Serial.println("SwitchRepository::loadSwitchData - switchOffCommand = " + String(switchData.switchOffCommand));
    Serial.println("SwitchRepository::loadSwitchData - isTriState = " + String(switchData.isTriState));
    Serial.println("SwitchRepository::loadSwitchData - pulseLength = " + String(switchData.pulseLength));
    Serial.println("SwitchRepository::loadSwitchData - repeatTransmit = " + String(switchData.repeatTransmit));
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
    Serial.println("SwitchRepository::saveSwitchData - pulseLength = " + String(switchDataParams.pulseLength));
    Serial.println("SwitchRepository::saveSwitchData - repeatTransmit = " + String(switchDataParams.repeatTransmit));

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

std::unordered_map<int, SwitchReceiveParams> MszSwitchRepository::loadSwitchReceiveData()
{
    Serial.println("SwitchRepository::loadSwitchReceiveData - enter");

    // We return a hashmap, the key of the items is the decimal from the 
    // radio switch. The value contains the MQTT topic data in the SwitchReceiveParams struct.
    std::unordered_map<int, SwitchReceiveParams> receiveParams;
    
    // Load the whole file with a maximum of SWITCH_MAX_RECEIVE_ENTRIES entries.
    File file = SPIFFS.open(SWITCH_FILENAME_RECEIVE_FILENAME, "r");
    if (file)
    {
        while (file.available())
        {
            SwitchReceiveParams receiveParam;
            file.readBytes((char *)&receiveParam, sizeof(receiveParam));
            if ((receiveParam.switchReceiveDecimalValue >= 0))
            {
                receiveParams[receiveParam.switchReceiveDecimalValue] = receiveParam;
            }
            else
            {
                Serial.println("SwitchRepository::loadSwitchReceiveData - invalid switchReceiveDecimalValue");
            }
        }
        file.close();
    }
    else
    {
        Serial.println("SwitchRepository::loadSwitchReceiveData - failed to open file");
    }

    Serial.println("SwitchRepository::loadSwitchReceiveData - exit");
    return receiveParams;
}

bool MszSwitchRepository::saveSwitchReceiveData (std::unordered_map<int, SwitchReceiveParams> receiveParams)
{
    Serial.println("SwitchRepository::saveSwitchReceiveData - enter");

    // Only write if there are not too many entries.
    if (receiveParams.size() > SWITCH_MAX_RECEIVE_ENTRIES)
    {
        Serial.println("SwitchRepository::saveSwitchReceiveData - too many entries");
        return false;
    }

    // Now write the contents to the file.
    bool succeeded = false;
    File file = SPIFFS.open(SWITCH_FILENAME_RECEIVE_FILENAME, "w");
    if (file)
    {
        for (auto it = receiveParams.begin(); it != receiveParams.end(); ++it)
        {
            Serial.println("SwitchRepository::saveSwitchReceiveData - key = " + String(it->first) + " val = " + String(it->second.switchCommand));
            if(file.write((const uint8_t *)&it->second, sizeof(it->second)) != sizeof(it->second))
            {
                Serial.println("SwitchRepository::saveSwitchReceiveData - failed to write data");
                break;
            }
        }
        file.close();
        succeeded = true;
    }
    else
    {
        Serial.println("SwitchRepository::saveSwitchReceiveData - failed to open file for writing data");
    }

    Serial.println("SwitchRepository::saveSwitchReceiveData - exit");
    return succeeded;
}

#endif