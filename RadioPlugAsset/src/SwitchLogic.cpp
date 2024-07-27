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
    // Configure the RCSwitch library
    pinMode(RCSWITCH_RECEIVE_PORT, INPUT);
    rcHandler.enableReceive(RCSWITCH_RECEIVE_PORT);
    rcHandler.enableTransmit(RCSWITCH_SEND_PORT);
    rcHandler.setPulseLength(RCSWITCH_DATA_PULSE_LENGTH);
    rcHandler.setProtocol(RCSWITCH_DATA_PROTOCOL);
    rcHandler.setRepeatTransmit(RCSWITCH_REPEAT_TRANSMIT);

    // Keep a reference to the WifiClient
    this->wifiClient = wifiClient;
}

/*
 * Switch handling methods (turning switches on and off).
 */
void MszSwitchLogic::handleSwitchReceiveData()
{
    //Serial.println("MszSwitchLogic::handleSwitchReceiveData - enter");

    if (rcHandler.available())
    {
        unsigned long receivedValue = rcHandler.getReceivedValue();
        unsigned int receivedProtocol = rcHandler.getReceivedProtocol();
        Serial.println("MszSwitchLogic::handleSwitchReceiveData - Received " + String(receivedValue) + " / " + String(receivedProtocol) + "bit");
        
        rcHandler.resetAvailable();

        MszSwitchRepository switchRepository;
        std::unordered_map<int, SwitchReceiveParams> savedReceiveParams = switchRepository.loadSwitchReceiveData();
        if (savedReceiveParams.find(receivedValue) != savedReceiveParams.end())
        {
            Serial.println("MszSwitchLogic::handleSwitchReceiveData - Found entry for: " + String(receivedValue));
            SwitchReceiveParams receiveParams = savedReceiveParams[receivedValue];

            if (receivedProtocol != receiveParams.switchProtocol)
            {
                Serial.println("MszSwitchLogic::handleSwitchReceiveData - Protocol mismatch: " + String(receivedProtocol) + " / " + String(receiveParams.switchProtocol));
                return;
            }

            // Next send the MQTT message per the receive parameters configuration and the global metadata configuration.
            AssetMetadataParams assetMetadata = switchRepository.loadMetadata();
            if (strnlen(receiveParams.switchTopic, MAX_SWITCH_MQTT_TOPIC_LENGTH) > 0)
            {
                Serial.println("MszSwitchLogic::handleSwitchReceiveData - Sending " + String(receiveParams.switchCommand) + " to MQTT topic " + String(receiveParams.switchCommand) + " on MQTT Server " + assetMetadata.sensorMqttServer);

                if(assetMetadata.sensorMqttServer != NULL && assetMetadata.sensorMqttServer[0] != '\0')
                {
                    // First connect to the MQTT server
                    int failedConnectionAttempts = 0;
                    PubSubClient mqttClient(wifiClient);
                    mqttClient.setServer(assetMetadata.sensorMqttServer, assetMetadata.sensorMqttPort);
                    while(!mqttClient.connected())
                    {
                        Serial.println("MszSwitchLogic::handleSwitchReceiveData - Connecting to MQTT server");
                        if(mqttClient.connect(assetMetadata.sensorMqttUsername, assetMetadata.sensorMqttUsername, assetMetadata.sensorMqttPassword))
                        {
                            Serial.println("MszSwitchLogic::handleSwitchReceiveData - Connected to MQTT server");
                            // Send this to MQTT server assetMetadata.sensorMqttServer with the topic receiveParams.switchTopic and the content receiveParams.switchCommand
                            mqttClient.publish(receiveParams.switchTopic, receiveParams.switchCommand);
                            Serial.println("MszSwitchLogic::handleSwitchReceiveData - Sent MQTT message");
                            break;
                        }
                        else if (failedConnectionAttempts >= SWITCH_MAX_MQTTCONNECT_ATTEMPTS)
                        {
                            Serial.println("MszSwitchLogic::handleSwitchReceiveData - Failed to connect to MQTT server after " + String(failedConnectionAttempts) + " attempts");
                            break;
                        }
                        else
                        {
                            failedConnectionAttempts++;
                            Serial.println("MszSwitchLogic::handleSwitchReceiveData - Failed to connect to MQTT server");
                            delay(500 + ((failedConnectionAttempts - 1) * 1000));
                        }
                    }

                    // Disconnect from the MQTT server. I know this is the most efficient way, but I will need to optimize later when
                    // everytying is working.
                    if(mqttClient.connected())
                    {
                        mqttClient.disconnect();
                        Serial.println("MszSwitchLogic::handleSwitchReceiveData - Disconnected from MQTT server");
                    }
                }
                else
                {
                    Serial.println("MszSwitchLogic::handleSwitchReceiveData - No MQTT server configured");
                }
            }
        }
        else
        {
            Serial.println("MszSwitchLogic::handleSwitchReceiveData - No entry found for: " + String(receivedValue));
        }
    }

    //Serial.println("MszSwitchLogic::handleSwitchReceiveData - exit");
}

int MszSwitchLogic::toggleSwitch(String switchName, bool switchOn)
{
    SwitchDataParams switchParams;

    Serial.println("MszSwitchLogic::toggleSwitch - enter");

    MszSwitchRepository switchRepository;
    SwitchDataParams switchData = switchRepository.loadSwitchData(switchName);
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