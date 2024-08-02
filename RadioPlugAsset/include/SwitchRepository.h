#ifndef MSZ_SWITCHREPOSITORY_H
#define MSZ_SWITCHREPOSITORY_H

#include <unordered_map>
#include <AssetApiBaseData.h>
#include <SwitchData.h>
#include "SwitchData.h"

class MszSwitchRepository
: public AssetBaseRepository
{
public:
  MszSwitchRepository();

  static constexpr const char *SWITCH_FILENAME_PREFIX = "/swf";
  static constexpr const char *SWITCH_FILENAME_RECEIVE_FILENAME = "/swr";

  static const int SWITCH_MAX_RECEIVE_ENTRIES = 32;

public:
  SwitchDataParams loadSwitchData(String switchName);
  bool saveSwitchData(String switchName, SwitchDataParams switchDataParams);

  std::unordered_map<int, SwitchReceiveParams> loadSwitchReceiveData();
  bool saveSwitchReceiveData(std::unordered_map<int, SwitchReceiveParams> receiveParams);
};

#endif // MSZ_SWITCHREPOSITORY_H