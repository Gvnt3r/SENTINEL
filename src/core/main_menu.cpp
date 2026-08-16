#include "main_menu.h"
#include "display.h"
#include "utils.h"
#include <globals.h>

#if defined(M5STICK)
namespace {
void drawStickAppIcon(const String &name, int x, int y) {
    uint16_t c = bruceConfig.priColor;
    tft.fillRoundRect(x - 19, y - 19, 38, 38, 8, getColorVariation(bruceConfig.secColor));
    tft.drawRoundRect(x - 19, y - 19, 38, 38, 8, c);

    if (name == "Automotive") {
        tft.drawRoundRect(x - 13, y - 6, 26, 13, 4, c);
        tft.drawLine(x - 8, y - 6, x - 3, y - 12, c);
        tft.drawLine(x - 3, y - 12, x + 7, y - 12, c);
        tft.drawLine(x + 7, y - 12, x + 11, y - 6, c);
        tft.fillCircle(x - 8, y + 8, 3, c);
        tft.fillCircle(x + 8, y + 8, 3, c);
    } else if (name == "WiFi") {
        tft.drawArc(x, y + 9, 15, 12, 210, 330, c, getColorVariation(bruceConfig.secColor));
        tft.drawArc(x, y + 9, 10, 7, 210, 330, c, getColorVariation(bruceConfig.secColor));
        tft.fillCircle(x, y + 8, 2, c);
    } else if (name == "BLE") {
        tft.drawLine(x, y - 14, x, y + 14, c);
        tft.drawLine(x, y - 14, x + 9, y - 6, c);
        tft.drawLine(x + 9, y - 6, x - 8, y + 8, c);
        tft.drawLine(x - 8, y - 8, x + 9, y + 6, c);
        tft.drawLine(x + 9, y + 6, x, y + 14, c);
    } else if (name == "RF" || name == "NRF24" || name == "LoRa" || name == "FM") {
        tft.fillCircle(x, y, 3, c);
        tft.drawArc(x, y, 10, 8, 300, 60, c, getColorVariation(bruceConfig.secColor));
        tft.drawArc(x, y, 16, 13, 300, 60, c, getColorVariation(bruceConfig.secColor));
        tft.drawArc(x, y, 10, 8, 120, 240, c, getColorVariation(bruceConfig.secColor));
        tft.drawArc(x, y, 16, 13, 120, 240, c, getColorVariation(bruceConfig.secColor));
    } else if (name == "RFID" || name == "Proxmark") {
        tft.drawRoundRect(x - 13, y - 10, 26, 20, 3, c);
        tft.drawFastVLine(x - 7, y - 5, 10, c);
        tft.drawFastVLine(x - 2, y - 5, 10, c);
        tft.drawFastHLine(x + 3, y - 5, 7, c);
        tft.drawFastHLine(x + 3, y, 7, c);
        tft.drawFastHLine(x + 3, y + 5, 7, c);
    } else if (name == "Files") {
        tft.drawRoundRect(x - 14, y - 8, 28, 19, 3, c);
        tft.fillRect(x - 11, y - 13, 12, 6, c);
    } else if (name == "Config") {
        tft.drawCircle(x, y, 11, c);
        tft.drawCircle(x, y, 4, c);
        tft.drawFastHLine(x - 16, y, 32, c);
        tft.drawFastVLine(x, y - 16, 32, c);
    } else {
        String mark = name.substring(0, name.length() > 3 ? 3 : name.length());
        mark.toUpperCase();
        tft.setTextSize(FP);
        tft.setTextColor(c, getColorVariation(bruceConfig.secColor));
        tft.drawCentreString(mark, x, y - 4, 1);
    }
}

void drawStickHome() {
    int selected = 0;
    for (int i = 0; i < static_cast<int>(options.size()); i++) {
        if (options[i].hovered) {
            selected = i;
            break;
        }
    }

    tft.fillScreen(bruceConfig.bgColor);
    tft.setTextSize(FP);
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.drawString("M5 SENTINEL", 8, 8, 1);
    tft.setTextColor(bruceConfig.secColor, bruceConfig.bgColor);
    String state = sdcardMounted ? "SD " : "-- ";
    state += String(getBattery()) + "%";
    tft.drawRightString(state, tftWidth - 8, 8, 1);
    tft.drawFastHLine(8, 23, tftWidth - 16, getColorVariation(bruceConfig.priColor));

    int previous = (selected - 1 + options.size()) % options.size();
    int next = (selected + 1) % options.size();

    // The rail makes the direction explicit: applications are stacked vertically.
    tft.drawFastVLine(tftWidth - 22, 31, 82, getColorVariation(bruceConfig.secColor));
    tft.fillTriangle(tftWidth - 27, 39, tftWidth - 17, 39, tftWidth - 22, 32, bruceConfig.secColor);
    tft.fillTriangle(tftWidth - 27, 105, tftWidth - 17, 105, tftWidth - 22, 112, bruceConfig.secColor);
    tft.fillCircle(tftWidth - 22, 72, 5, bruceConfig.priColor);

    tft.setTextSize(FP);
    tft.setTextColor(getColorVariation(bruceConfig.secColor), bruceConfig.bgColor);
    tft.drawString(options[previous].label.substring(0, 18), 12, 31, 1);
    tft.drawString(options[next].label.substring(0, 18), 12, 103, 1);

    String label = options[selected].label;
    drawStickAppIcon(label, 32, 70);
    int titleSize = label.length() <= 10 ? FG : FM;
    tft.setTextSize(titleSize);
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.drawString(label.substring(0, 15), 58, 61 - titleSize * 2, 1);
    tft.fillRoundRect(58, 87, 92, 17, 5, bruceConfig.priColor);
    tft.setTextSize(FP);
    tft.setTextColor(bruceConfig.bgColor, bruceConfig.priColor);
    tft.drawCentreString("M5  OUVRIR", 104, 91, 1);

    tft.setTextColor(bruceConfig.secColor, bruceConfig.bgColor);
    tft.drawRightString(String(selected + 1) + " / " + String(options.size()), tftWidth - 34, 91, 1);
}
} // namespace
#endif

MainMenu::MainMenu() {
    _menuItems = {
        &automotiveMenu,
        &wifiMenu,
        &bleMenu,
        &rfMenu,
        &nrf24Menu,
#if !defined(LITE_VERSION)
        &loraMenu,
#endif
#if defined(FM_SI4713) && !defined(LITE_VERSION)
        &fmMenu,
#endif
        &irMenu,
#if !defined(LITE_VERSION)
        &ethernetMenu,
#endif
        &gpsMenu,
        &rfidMenu,
        &proxmarkMenu,
        &fileMenu,
#if !defined(LITE_VERSION) && !defined(DISABLE_INTERPRETER)
        &scriptsMenu,
#endif
        &clockMenu,
        &othersMenu,
        &configMenu,
    };

    _totalItems = _menuItems.size();
}

MainMenu::~MainMenu() {}

void MainMenu::begin(void) {
    returnToMenu = false;
    options = {};

    std::vector<String> l = bruceConfig.disabledMenus;
    for (int i = 0; i < _totalItems; i++) {
        String itemName = _menuItems[i]->getName();
        if (find(l.begin(), l.end(), itemName) == l.end()) { // If menu item is not disabled
            options.push_back(
                {// selected lambda
                 itemName,
                 [this, i]() { _menuItems[i]->optionsMenu(); },
                 false,                                  // selected = false
                 [](void *menuItem, bool shouldRender) { // render lambda
                     if (!shouldRender) return false;
#if defined(M5STICK)
                     drawStickHome();
#else
                     drawMainBorder(false);

                     MenuItemInterface *obj = static_cast<MenuItemInterface *>(menuItem);
                     float scale = float((float)tftWidth / (float)240);
                     if (bruceConfigPins.rotation & 0b01) scale = float((float)tftHeight / (float)135);
                     obj->draw(scale);
#endif
#if defined(HAS_TOUCH)
                     TouchFooter();
#endif
                     return true;
                 },
                 _menuItems[i]
                }
            );
        }
    }
    _currentIndex = loopOptions(options, MENU_TYPE_MAIN, "Main Menu", _currentIndex);
};

/*********************************************************************
**  Function: hideAppsMenu
**  Menu to Hide or show menus
**********************************************************************/

void MainMenu::hideAppsMenu() {
    auto items = this->getItems();
    int index = 0;
RESTART: // using gotos to avoid stackoverflow after many choices
    options.clear();
    for (auto item : items) {
        String label = item->getName();
        std::vector<String> l = bruceConfig.disabledMenus;
        bool enabled = find(l.begin(), l.end(), label) == l.end();
        options.push_back(
            {label,
             [this, label, enabled]() {
                 if (enabled) bruceConfig.addDisabledMenu(label);
                 else bruceConfig.removeDisabledMenu(label);
             },
             enabled}
        );
    }
    options.push_back({"Show All", [=]() { bruceConfig.disabledMenus.clear(); }, true});
    addOptionToMainMenu();
    index = loopOptions(options, index);
    bruceConfig.saveFile();
    if (!returnToMenu) goto RESTART;
}
