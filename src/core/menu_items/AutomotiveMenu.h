#ifndef __AUTOMOTIVE_MENU_H__
#define __AUTOMOTIVE_MENU_H__

#include <MenuItemInterface.h>

class AutomotiveMenu : public MenuItemInterface {
public:
    AutomotiveMenu() : MenuItemInterface("Automotive") {}
    void optionsMenu(void) override;
    void drawIcon(float scale) override;
    bool hasTheme() override { return false; }
    const String &themePath() override {
        static const String empty;
        return empty;
    }
};

#endif
