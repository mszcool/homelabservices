#ifndef MSZ_SECRETHANDLER_H
#define MSZ_SECRETHANDLER_H

#include <Arduino.h>
#include <string>
#include <unordered_map>

class MszSecretHandler {
private:
  static const int MAX_SECRETS = 5;

  // Had memory issues on ESP32 with std::unordered_map, so using arrays instead.
  char *secrets[MszSecretHandler::MAX_SECRETS];

public:
  MszSecretHandler();
  ~MszSecretHandler();

  // Note: had to use return-codes for these functions below because ESP8266 does not support
  //       C++ exceptions and many aspects of the standard template library.
  char* getSecret(int index);
  bool setSecret(int index, const char *secret, int secretLength);

  bool validateTokenSignature(String token, long tokenTimestamp, String secretKey, String signature, int tokenExpirationSeconds);

private:
  String toHexString(const uint8_t *input, size_t length);
};

#endif //MSZ_SECRETHANDLER_H