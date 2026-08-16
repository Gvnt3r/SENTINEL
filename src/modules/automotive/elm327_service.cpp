#include "elm327_service.h"

#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/operation_control.h"
#include "core/utils.h"
#include <BluetoothSerial.h>

namespace Automotive {
namespace {
BluetoothSerial elmSerial;
Elm327Service sharedElm;

String compactHex(String value) {
    value.replace("SEARCHING...", ""); value.replace(" ", ""); value.replace("\r", ""); value.replace("\n", "");
    value.replace(">", ""); value.toUpperCase(); return value;
}

bool byteAt(const String &hex, size_t offset, uint8_t &value) {
    if (offset + 2 > hex.length()) return false;
    char pair[3] = {hex[offset], hex[offset + 1], 0}; char *end = nullptr;
    long parsed = strtol(pair, &end, 16); if (!end || *end) return false; value = parsed; return true;
}
}

bool Elm327Service::command(const String &request, String &response, uint32_t timeoutMs) {
    if (!_status.connected) { _status.lastError = "ELM327 non connecte"; return false; }
    while (elmSerial.available()) elmSerial.read();
    elmSerial.print(request); elmSerial.print('\r'); response = ""; uint32_t started = millis();
    while (millis() - started < timeoutMs) {
        if (operationExitRequested()) {
            _status.lastError = "Operation annulee";
            return false;
        }
        while (elmSerial.available()) {
            char c = elmSerial.read(); if (c == '>') return true;
            if (response.length() >= 4096) { _status.lastError = "Reponse ELM327 trop longue"; return false; }
            response += c;
        }
        delay(2);
    }
    _status.lastError = "Timeout ELM327: " + request; return false;
}

bool Elm327Service::initialize() {
    String reply;
    const char *setup[] = {"ATZ", "ATE0", "ATL0", "ATS0", "ATH0", "ATSP0"};
    for (const char *cmd : setup) if (!command(cmd, reply, cmd[2] == 'Z' ? 5000 : 2000)) return false;
    if (command("ATI", reply)) _status.adapter = reply;
    if (command("ATDP", reply)) _status.protocol = reply;
    return true;
}

bool Elm327Service::connect(const String &device, const String &pin) {
    disconnect(); _status.device = device.length() ? device : "OBDII";
    if (!elmSerial.begin("SENTINEL", true, true)) { _status.lastError = "Bluetooth Classic indisponible"; return false; }
    elmSerial.setPin(pin.c_str(), pin.length());
    if (!elmSerial.connect(_status.device) || !elmSerial.connected(10000)) {
        _status.lastError = "Connexion impossible: " + _status.device; elmSerial.end(); return false;
    }
    _status.connected = true;
    if (!initialize()) { disconnect(); return false; }
    return true;
}

void Elm327Service::disconnect() {
    if (_status.connected) elmSerial.disconnect();
    elmSerial.end();
    _status.connected = false;
}

bool Elm327Service::readPid(uint8_t pid, float &value, String &unit) {
    char cmd[5]; snprintf(cmd, sizeof(cmd), "01%02X", pid); String reply;
    if (!command(cmd, reply)) return false;
    String hex = compactHex(reply);
    String marker = String("41") + String(cmd + 2); int pos = hex.indexOf(marker); uint8_t a, b = 0;
    if (pos < 0 || !byteAt(hex, pos + 4, a)) { _status.lastError = "Reponse PID invalide"; return false; }
    byteAt(hex, pos + 6, b);
    switch (pid) {
        case 0x05: value = int(a) - 40; unit = "C"; break;
        case 0x0C: value = ((a * 256U) + b) / 4.0f; unit = "rpm"; break;
        case 0x0D: value = a; unit = "km/h"; break;
        case 0x11: value = a * 100.0f / 255.0f; unit = "%"; break;
        default: value = a; unit = "raw"; break;
    }
    return true;
}

bool Elm327Service::readDtcs(String &dtcs) {
    String reply; if (!command("03", reply, 5000)) return false; String hex = compactHex(reply); int pos = hex.indexOf("43");
    if (pos < 0) { _status.lastError = "Reponse DTC invalide"; return false; } dtcs = "";
    static const char families[] = "PCBU";
    for (size_t i = pos + 2; i + 3 < hex.length(); i += 4) { uint8_t a, b; if (!byteAt(hex, i, a) || !byteAt(hex, i + 2, b) || (!a && !b)) continue;
        char code[6]; snprintf(code, sizeof(code), "%c%01X%02X", families[(a >> 6) & 3], (a >> 4) & 3, ((a & 15) << 8) | b);
        if (dtcs.length()) dtcs += ' ';
        dtcs += code;
    }
    if (!dtcs.length()) dtcs = "Aucun";
    return true;
}

bool Elm327Service::clearDtcs(bool confirmed) {
    if (!confirmed) { _status.lastError = "Confirmation requise"; return false; }
    String reply; return command("04", reply, 5000);
}

Elm327Service &elm327() { return sharedElm; }

void elm327Screen() {
    String name = keyboard("OBDII", 32, "Nom Bluetooth ELM327:"); if (!name.length()) return;
    String pin = keyboard("1234", 16, "PIN ELM327:", true); if (!pin.length()) return;
    armOperationExit();
    drawMainBorderWithTitle("ELM327 OBD-II"); padprintln("Connexion " + name + "...");
    if (!sharedElm.connect(name, pin)) { displayError(sharedElm.status().lastError); return; }
    armOperationExit();
    while (!operationExitRequested()) {
        float rpm = 0, speed = 0, coolant = 0; String unit;
        sharedElm.readPid(0x0C, rpm, unit); sharedElm.readPid(0x0D, speed, unit); sharedElm.readPid(0x05, coolant, unit);
        drawMainBorderWithTitle("ELM327 DIAGNOSTIC");
        padprintln(sharedElm.status().adapter); padprintln(sharedElm.status().protocol);
        padprintln("RPM: " + String(rpm, 0)); padprintln("Vitesse: " + String(speed, 0) + " km/h");
        padprintln("Liquide: " + String(coolant, 0) + " C"); padprintln("2x bouton haut: retour"); delay(300);
    }
    sharedElm.disconnect();
}
} // namespace Automotive
