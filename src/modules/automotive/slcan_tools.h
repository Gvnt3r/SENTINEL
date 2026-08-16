#ifndef __SLCAN_TOOLS_H__
#define __SLCAN_TOOLS_H__

#include <Arduino.h>

namespace SlcanTools {

void dashboard(uint32_t bitrate);
void activeIds();
void capture(uint32_t bitrate);
void replay(uint32_t bitrate);
void wiringInfo();

} // namespace SlcanTools

#endif
