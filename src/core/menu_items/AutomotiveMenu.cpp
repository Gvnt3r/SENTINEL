#include "AutomotiveMenu.h"

#include "core/utils.h"
#include "modules/automotive/slcan_tools.h"

void AutomotiveMenu::optionsMenu() {
    options = {
        {"Dashboard", []() { SlcanTools::dashboard(500000); }},
        {"IDs actifs", SlcanTools::activeIds},
        {"Inspecteur ID", SlcanTools::activeIds},
        {"Capture ASC", []() { SlcanTools::capture(500000); }},
        {"Emission / Replay", []() { SlcanTools::replay(500000); }},
        {"Scenarios", []() { SlcanTools::replay(500000); }},
        {"Configuration", SlcanTools::wiringInfo},
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
