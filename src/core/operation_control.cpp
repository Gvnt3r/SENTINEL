#include "operation_control.h"

#include <Arduino.h>
#include <globals.h>

namespace {
bool topWasDown = false;
uint32_t lastTopClick = 0;
constexpr uint32_t DOUBLE_CLICK_MS = 550;
}

void armOperationExit() {
    EscPress = false;
    PrevPress = false;
    topWasDown = digitalRead(UP_BTN) == LOW;
    lastTopClick = 0;
}

bool operationExitRequested() {
    if (check(EscPress)) return true;

    const bool topDown = digitalRead(UP_BTN) == LOW;
    bool exit = false;
    if (topDown && !topWasDown) {
        const uint32_t now = millis();
        exit = lastTopClick && now - lastTopClick <= DOUBLE_CLICK_MS;
        lastTopClick = now;
    }
    topWasDown = topDown;

    // The operation owns the top button, so do not leak navigation events.
    PrevPress = false;
    return exit;
}
