#ifndef MSZ_SWITCHREPOSITORY_H
#define MSZ_SWITCHREPOSITORY_H

#include <SwitchData.h>
#include "SwitchData.h"

class MszSwitchRepository
{
public:
  MszSwitchRepository();

  static constexpr const char *SWITCH_METADATA_FILENAME = "/switchmetadata";
  static constexpr const char *SWITCH_FILENAME_PREFIX = "/switchfile";

public:
  SwitchMetadataParams loadMetadata();
  bool saveMetadata(SwitchMetadataParams metadata);
  SwitchDataParams loadSwitchData(String switchName);
  bool saveSwitchData(String switchName, SwitchDataParams switchDataParams);
};

#endif // MSZ_SWITCHREPOSITORY_H