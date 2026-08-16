#include "AutomotiveMenu.h"

#include "core/utils.h"
#include "modules/automotive/can_suite.h"
#include "modules/automotive/elm327_service.h"
#include "modules/automotive/slcan_tools.h"

namespace {
uint32_t canBitrate = 500000;

void selectBitrate() {
    std::vector<Option> rates = {
        {"10 kbit/s", []() { canBitrate = 10000; }}, {"20 kbit/s", []() { canBitrate = 20000; }},
        {"50 kbit/s", []() { canBitrate = 50000; }}, {"100 kbit/s", []() { canBitrate = 100000; }},
        {"125 kbit/s", []() { canBitrate = 125000; }}, {"250 kbit/s", []() { canBitrate = 250000; }},
        {"500 kbit/s", []() { canBitrate = 500000; }}, {"800 kbit/s", []() { canBitrate = 800000; }},
        {"1 Mbit/s", []() { canBitrate = 1000000; }},
    };
    loopOptions(rates, MENU_TYPE_SUBMENU, "Debit CAN");
}

void canConfiguration() {
    std::vector<Option> config = {
        {String("Backend: ") + Automotive::backendName(Automotive::backend()), []() {
            Automotive::setBackend(Automotive::backend() == Automotive::CanBackend::TWAI
                                       ? Automotive::CanBackend::SLCAN : Automotive::CanBackend::TWAI);
        }},
        {String("Debit: ") + String(canBitrate / 1000) + " kbit/s", selectBitrate},
        {"Verrouiller TX", Automotive::lockTransmission},
        {"Cablage / securite", SlcanTools::wiringInfo},
    };
    loopOptions(config, MENU_TYPE_SUBMENU, "Configuration CAN");
}
}

void AutomotiveMenu::optionsMenu() {
    options = {
        {"Dashboard", []() { SlcanTools::dashboard(canBitrate); }},
        {"IDs actifs", SlcanTools::activeIds},
        {"Inspecteur ID", SlcanTools::activeIds},
        {"Capture ASC", []() { SlcanTools::capture(canBitrate); }},
        {"Emission / Replay", []() { SlcanTools::replay(canBitrate); }},
        {"Scenarios", []() { SlcanTools::replay(canBitrate); }},
        {"ELM327 OBD-II", Automotive::elm327Screen},
        {"Configuration", canConfiguration},
    };
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "Automotive");
}

void AutomotiveMenu::drawIcon(float scale) {
    clearIconArea();
    int x = iconCenterX;
    int y = iconCenterY;
    int w = 66 * scale;
    int h = 25 * scale;
    tft.drawRoundRect(x - w / 2, y - h / 2, w, h, 6 * scale, bruceConfig.priColor);
    tft.drawLine(x - w / 3, y - h / 2, x - w / 5, y - h, bruceConfig.priColor);
    tft.drawLine(x - w / 5, y - h, x + w / 4, y - h, bruceConfig.priColor);
    tft.drawLine(x + w / 4, y - h, x + w / 3, y - h / 2, bruceConfig.priColor);
    tft.fillCircle(x - w / 4, y + h / 2, 7 * scale, bruceConfig.bgColor);
    tft.drawCircle(x - w / 4, y + h / 2, 7 * scale, bruceConfig.priColor);
    tft.fillCircle(x + w / 4, y + h / 2, 7 * scale, bruceConfig.bgColor);
    tft.drawCircle(x + w / 4, y + h / 2, 7 * scale, bruceConfig.priColor);
}
