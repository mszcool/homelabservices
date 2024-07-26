#include <functional>
#include <sstream>
#include <iomanip>
#include <unordered_map>

#include "SwitchLogic.h"

/*
 * Constructors take care about RCSwitch initialization.
 */
MszSwitchLogic::MszSwitchLogic()
{
    rcHandler.enableTransmit(RCSWITCH_SEND_PORT);
    rcHandler.setPulseLength(RCSWITCH_DATA_PULSE_LENGTH);
    rcHandler.setProtocol(RCSWITCH_DATA_PROTOCOL);
    rcHandler.setRepeatTransmit(RCSWITCH_REPEAT_TRANSMIT);
}

/*
 * Switch handling methods (turning switches on and off).
 */
void MszSwitchLogic::handleSwitchReceiveData()
{
    Serial.println("MszSwitchLogic::handleSwitchReceiveData - enter");

    if (rcHandler.available())
    {
        unsigned long receivedValue = rcHandler.getReceivedValue();
        unsigned int receivedProtocol = rcHandler.getReceivedProtocol();
        Serial.println("MszSwitchLogic::handleSwitchReceiveData - Received " + String(receivedValue) + " / " + String(rcHandler.getReceivedBitlength()) + "bit");
        
        std::unordered_map<int, SwitchReceiveParams> savedReceiveParams = this->switchRepository.loadSwitchReceiveData();
        if (savedReceiveParams.find(receivedValue) != savedReceiveParams.end())
        {
            Serial.println("MszSwitchLogic::handleSwitchReceiveData - Found entry for: " + String(receivedValue));
            SwitchReceiveParams receiveParams = savedReceiveParams[receivedValue];

            // Next send the MQTT message per the receive parameters configuration and the global metadata configuration.
            AssetMetadataParams assetMetadata = assetRepository.loadMetadata();
            if (strnlen(receiveParams.switchTopic, MAX_SWITCH_MQTT_TOPIC_LENGTH) > 0)
            {
                Serial.println("MszSwitchLogic::handleSwitchReceiveData - Sending " + String(receiveParams.switchCommand) + " to MQTT topic " + String(receiveParams.switchCommand) + " on MQTT Server " + assetMetadata.sensorMqttServer);

                // Send this to MQTT server assetMetadata.sensorMqttServer with the topic receiveParams.switchTopic and the content receiveParams.switchCommand
                // TODO: Implement MQTT sending
            }
        }
        else
        {
            Serial.println("MszSwitchLogic::handleSwitchReceiveData - No entry found for: " + String(receivedValue));
        }
    }

    Serial.println("MszSwitchLogic::handleSwitchReceiveData - exit");
}

int MszSwitchLogic::toggleSwitch(String switchName, bool switchOn)
{
    SwitchDataParams switchParams;

    Serial.println("MszSwitchLogic::toggleSwitch - enter");

    SwitchDataParams switchData = this->switchRepository.loadSwitchData(switchName);
    if (strnlen(switchData.switchName, MAX_SWITCH_NAME_LENGTH) == 0)
    {
        Serial.println("MszSwitchLogic::toggleSwitch - enter - switch not found");
        Serial.println("MszSwitchLogic::toggleSwitch - exit");
        return SWITCH_TOGGLE_NOTFOUND;
    }
    else
    {
        rcHandler.enableTransmit(RCSWITCH_SEND_PORT);
        rcHandler.setProtocol(switchData.switchProtocol);
        rcHandler.setPulseLength(switchData.pulseLength > 0 ? switchData.pulseLength : RCSWITCH_DATA_PULSE_LENGTH);
        rcHandler.setRepeatTransmit(switchData.repeatTransmit > 0 ? switchData.repeatTransmit : RCSWITCH_REPEAT_TRANSMIT);
        if (switchData.isTriState)
        {
            rcHandler.sendTriState((switchOn ? switchData.switchOnCommand : switchData.switchOffCommand));
        }
        else
        {
            rcHandler.send((switchOn ? switchData.switchOnCommand : switchData.switchOffCommand));
        }
        Serial.println("MszSwitchLogic::toggleSwitch - exit");
        return switchOn ? SWITCH_TOGGLE_SWITCHEDON : SWITCH_TOGGLE_SWITCHEDOFF;
    }
}