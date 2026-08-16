#ifndef __PROXMARK_BRIDGE_H__
#define __PROXMARK_BRIDGE_H__

#include <Arduino.h>

namespace ProxmarkBridge {

constexpr uint32_t DEFAULT_BAUD = 115200;

void run(uint32_t baud = DEFAULT_BAUD, bool logToSd = false);
void wiringInfo();

} // namespace ProxmarkBridge

#endif
