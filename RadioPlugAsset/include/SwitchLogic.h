#ifndef MSZ_SWITCHLOGIC_H
#define MSZ_SWITCHLOGIC_H

#include <Arduino.h>
#include <RCSwitch.h>
#include <SecretHandler.h>
#include "SwitchData.h"
#include "AssetApiBaseData.h"
#include "SwitchRepository.h"

class MszSwitchLogic
{
public:
    MszSwitchLogic();

    void handleSwitchReceiveData();
    int toggleSwitch(String switchName, bool switchOn);

    static const int RCSWITCH_RECEIVE_PORT = 19;
    static const int RCSWITCH_SEND_PORT = 23;
    static const int RCSWITCH_DATA_PULSE_LENGTH = 512;
    static const int RCSWITCH_DATA_PROTOCOL = 5;
    static const int RCSWITCH_REPEAT_TRANSMIT = 10;
    static const int RCSWITCH_BIT_LENGTH = 24;

    static const int SWITCH_TOGGLE_SWITCHEDON = 1;
    static const int SWITCH_TOGGLE_SWITCHEDOFF = 0;
    static const int SWITCH_TOGGLE_NOTFOUND = -1;

protected:
    RCSwitch rcHandler;

private:
    AssetBaseRepository assetRepository;
    MszSwitchRepository switchRepository;
};

#endif // MSZ_SWITCHLOGIC_H