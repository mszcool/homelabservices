#ifndef SWITCHDATA_H
#define SWITCHDATA_H

#include <Arduino.h>

#define MAX_SWITCH_NAME_LENGTH 64
#define MAX_SWITCH_COMMAND_LENGTH 64

/// @brief Defines the parameters for the Switch
/// @details Defines a unique ID for the switch such that the config can be updated, a name, and the command.
///          If isTriState is true, the command is a Tristate while if false, it is a decimal.
///          switchCommand contains either the binary decimal command or the tristate command.
struct SwitchDataParams
{
  bool isTriState;
  int switchProtocol;
  char switchName[MAX_SWITCH_NAME_LENGTH+1];
  char switchOnCommand[MAX_SWITCH_COMMAND_LENGTH+1];
  char switchOffCommand[MAX_SWITCH_COMMAND_LENGTH+1];
  int pulseLength;
  int repeatTransmit;
};

#endif // SWITCHDATA_H