#include "ProxmarkMenu.h"

#include "core/utils.h"
#include "modules/proxmark/proxmark_bridge.h"

void ProxmarkMenu::optionsMenu() {
    options = {
        {"UART Bridge", []() { ProxmarkBridge::run(115200, false); }},
        {"Bridge + SD log", []() { ProxmarkBridge::run(115200, true); }},
        {"Wiring", ProxmarkBridge::wiringInfo},
    };
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "Proxmark");
}

void ProxmarkMenu::drawIcon(float scale) {
    clearIconArea();
    int w = 58 * scale;
    int h = 34 * scale;
    int x = iconCenterX - w / 2;
    int y = iconCenterY - h / 2;
    tft.drawRoundRect(x, y, w, h, 5 * scale, bruceConfig.priColor);
    tft.fillCircle(x + 13 * scale, iconCenterY, 4 * scale, bruceConfig.priColor);
    tft.drawLine(x + 24 * scale, y + 8 * scale, x + 24 * scale, y + h - 8 * scale, bruceConfig.priColor);
    tft.drawLine(x + 32 * scale, y + 10 * scale, x + w - 7 * scale, y + 10 * scale, bruceConfig.priColor);
    tft.drawLine(x + 32 * scale, iconCenterY, x + w - 7 * scale, iconCenterY, bruceConfig.priColor);
    tft.drawLine(x + 32 * scale, y + h - 10 * scale, x + w - 7 * scale, y + h - 10 * scale, bruceConfig.priColor);
}
