#pragma once

#include <Arduino.h>

namespace Automotive {

struct Elm327Status {
    bool connected = false;
    String device;
    String adapter;
    String protocol;
    String lastError;
};

class Elm327Service {
public:
    bool connect(const String &device, const String &pin = "1234");
    void disconnect();
    bool command(const String &request, String &response, uint32_t timeoutMs = 2500);
    bool readPid(uint8_t pid, float &value, String &unit);
    bool readDtcs(String &dtcs);
    bool clearDtcs(bool confirmed);
    const Elm327Status &status() const { return _status; }

private:
    bool initialize();
    Elm327Status _status;
};

Elm327Service &elm327();
void elm327Screen();

} // namespace Automotive
