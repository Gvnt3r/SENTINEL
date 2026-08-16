#ifndef __PROXMARK_MENU_H__
#define __PROXMARK_MENU_H__

#include <MenuItemInterface.h>

class ProxmarkMenu : public MenuItemInterface {
public:
    ProxmarkMenu() : MenuItemInterface("Proxmark") {}

    void optionsMenu(void) override;
    void drawIcon(float scale) override;
    bool hasTheme() override { return false; }
    const String &themePath() override {
        static const String empty;
        return empty;
    }
};

#endif
