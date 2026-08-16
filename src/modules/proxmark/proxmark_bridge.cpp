#include "proxmark_bridge.h"

#include "core/display.h"
#include "core/sd_functions.h"
#include <SD.h>
#include <globals.h>

namespace {
HardwareSerial proxmarkSerial(1);

void drawBridgeScreen(uint32_t baud, bool logging, uint32_t hostToPm3, uint32_t pm3ToHost) {
    drawMainBorderWithTitle("Proxmark UART");
    tft.setTextSize(1);
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.setCursor(12, 36);
    tft.printf("UART: %lu 8N1\n", static_cast<unsigned long>(baud));
    tft.printf("TX GPIO: %d\n", BAD_TX);
    tft.printf("RX GPIO: %d\n", BAD_RX);
    tft.printf("PC -> PM3: %lu B\n", static_cast<unsigned long>(hostToPm3));
    tft.printf("PM3 -> PC: %lu B\n", static_cast<unsigned long>(pm3ToHost));
    tft.printf("SD log: %s\n", logging ? "ON" : "OFF");
    tft.println("M5+Power: quitter");
}
} // namespace

namespace ProxmarkBridge {

void wiringInfo() {
    drawMainBorderWithTitle("Proxmark cablage");
    tft.setTextSize(1);
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.setCursor(10, 34);
    tft.println("M5 G32 TX -> PM3 RX");
    tft.println("M5 G33 RX <- PM3 TX");
    tft.println("M5 GND    -- PM3 GND");
    tft.println("UART 3.3 V uniquement");
    tft.println("Alimenter PM3 separement");
    tft.println("SD: 14/0/25/26");
    tft.println("M5+Power: retour");
    while (!check(EscPress) && !check(SelPress)) delay(20);
}

void run(uint32_t baud, bool logToSd) {
    File log;
    bool logging = false;
    if (logToSd && setupSdCard()) {
        SD.mkdir("/Proxmark");
        log = SD.open("/Proxmark/uart_capture.bin", FILE_APPEND);
        logging = static_cast<bool>(log);
    }

    // Grove: GPIO32 is M5 TX and GPIO33 is M5 RX. UART0 remains the USB host side.
    proxmarkSerial.begin(baud, SERIAL_8N1, BAD_RX, BAD_TX);
    proxmarkSerial.setRxBufferSize(4096);

    uint32_t hostToPm3 = 0;
    uint32_t pm3ToHost = 0;
    uint32_t lastDraw = 0;
    drawBridgeScreen(baud, logging, hostToPm3, pm3ToHost);

    while (!check(EscPress)) {
        while (Serial.available()) {
            uint8_t value = static_cast<uint8_t>(Serial.read());
            proxmarkSerial.write(value);
            hostToPm3++;
            if (logging) log.write(value);
        }
        while (proxmarkSerial.available()) {
            uint8_t value = static_cast<uint8_t>(proxmarkSerial.read());
            Serial.write(value);
            pm3ToHost++;
            if (logging) log.write(value);
        }
        if (millis() - lastDraw >= 500) {
            lastDraw = millis();
            if (logging) log.flush();
            drawBridgeScreen(baud, logging, hostToPm3, pm3ToHost);
        }
        delay(1);
    }

    if (logging) log.close();
    proxmarkSerial.end();
}

} // namespace ProxmarkBridge
