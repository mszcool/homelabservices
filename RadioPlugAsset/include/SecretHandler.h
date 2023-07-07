#ifndef MSZ_SECRETHANDLER_H
#define MSZ_SECRETHANDLER_H

#include <Arduino.h>

class MszSecretHandler {
public:
  MszSecretHandler();
  String getSecret(String secretName);
  void writeSecret(String secretName, String secret);
};

#endif //MSZ_SECRETHANDLER_H