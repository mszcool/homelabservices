#ifndef MSZ_SWITCHREPOSITORY_H
#define MSZ_SWITCHREPOSITORY_H

#include <AssetApiBaseData.h>
#include <SwitchData.h>
#include "SwitchData.h"

class MszSwitchRepository
: public AssetBaseRepository
{
public:
  MszSwitchRepository();

  static constexpr const char *SWITCH_FILENAME_PREFIX = "/swf";

public:
  SwitchDataParams loadSwitchData(String switchName);
  bool saveSwitchData(String switchName, SwitchDataParams switchDataParams);
};

#endif // MSZ_SWITCHREPOSITORY_H