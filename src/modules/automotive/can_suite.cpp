#include "can_suite.h"

#include "core/sd_functions.h"
#include <ArduinoJson.h>
#include <SD.h>
#include <Preferences.h>
#include "core/grove_resource.h"
#include <ctype.h>
#include <driver/twai.h>

namespace Automotive {
namespace {
HardwareSerial canUart(1);
SlcanAdapter sharedAdapter(canUart);
TwaiTransport sharedTwai;
CanActivityTable sharedActivity;
CanBackend selectedBackend = CanBackend::TWAI;
bool txUnlocked = false;
uint32_t activeBitrate = 0;
int8_t activeRxPin = -1;
int8_t activeTxPin = -1;

int hexNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    c = toupper(static_cast<unsigned char>(c));
    return c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
}

bool safeName(const String &name) {
    if (!name.length() || name.length() > 48 || name == "." || name == "..") return false;
    for (size_t i = 0; i < name.length(); ++i)
        if (!(isalnum(static_cast<unsigned char>(name[i])) || name[i] == '-' || name[i] == '_')) return false;
    return true;
}

void setError(String *target, const String &value) { if (target) *target = value; }

bool jsonToFrame(JsonVariantConst json, CanFrame &frame, String *error) {
    if (!json["id"].is<uint32_t>() || !json["dlc"].is<uint8_t>()) {
        setError(error, "id/dlc manquant"); return false;
    }
    frame.id = json["id"].as<uint32_t>();
    frame.extended = json["extended"] | false;
    frame.rtr = json["rtr"] | false;
    frame.dlc = json["dlc"].as<uint8_t>();
    JsonArrayConst data = json["data"].as<JsonArrayConst>();
    if (!frame.rtr && data.size() != frame.dlc) { setError(error, "taille data != dlc"); return false; }
    for (size_t i = 0; i < data.size() && i < 8; ++i) {
        if (!data[i].is<uint8_t>()) { setError(error, "octet invalide"); return false; }
        frame.data[i] = data[i].as<uint8_t>();
    }
    return validateFrame(frame, error);
}
} // namespace

SlcanAdapter::SlcanAdapter(HardwareSerial &serial) : _serial(serial) {}

char SlcanAdapter::bitrateCode(uint32_t bitrate) {
    switch (bitrate) {
        case 10000: return '0'; case 20000: return '1'; case 50000: return '2';
        case 100000: return '3'; case 125000: return '4'; case 250000: return '5';
        case 500000: return '6'; case 800000: return '7'; case 1000000: return '8';
        default: return 0;
    }
}

bool SlcanAdapter::transact(const String &cmd, uint32_t timeoutMs) {
    while (_serial.available()) _serial.read();
    _serial.print(cmd); _serial.write('\r');
    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        while (_serial.available()) {
            char c = static_cast<char>(_serial.read());
            if (c == '\r') return true;
            if (c == '\a') { _errors++; _lastError = "SLCAN refuse: " + cmd; return false; }
        }
        delay(1);
    }
    _errors++; _lastError = "SLCAN timeout: " + cmd; return false;
}

bool SlcanAdapter::begin(uint32_t bitrate, int8_t rxPin, int8_t txPin) {
    return begin(bitrate, rxPin, txPin, false);
}

bool SlcanAdapter::begin(uint32_t bitrate, int8_t rxPin, int8_t txPin, bool listenOnly) {
    char code = bitrateCode(bitrate);
    if (!code) { _lastError = "Debit non supporte"; return false; }
    _connected = false; _line = "";
    _serial.begin(SLCAN_BAUD, SERIAL_8N1, rxPin, txPin);
    _serial.setRxBufferSize(4096);
    if (!transact("C") || !transact("S" + String(code)) || !transact(listenOnly ? "L" : "O")) {
        _serial.end(); return false;
    }
    _bitrate = bitrate; _connected = true; _listenOnly = listenOnly; _lastError = "";
    _rxFrames = 0; _txFrames = 0; return true;
}

void SlcanAdapter::end() {
    if (_connected) transact("C");
    _connected = false; _serial.end(); _line = "";
}

bool SlcanAdapter::parseFrame(const String &line, CanFrame &frame) {
    if (line.length() < 5) return false;
    char kind = line[0];
    frame.extended = kind == 'T' || kind == 'R';
    frame.rtr = kind == 'r' || kind == 'R';
    if (kind != 't' && kind != 'T' && kind != 'r' && kind != 'R') return false;
    size_t idLen = frame.extended ? 8 : 3;
    size_t dlcPos = 1 + idLen;
    if (line.length() <= dlcPos || line[dlcPos] < '0' || line[dlcPos] > '8') return false;
    frame.id = 0;
    for (size_t i = 1; i <= idLen; ++i) {
        int n = hexNibble(line[i]); if (n < 0) return false;
        frame.id = (frame.id << 4) | n;
    }
    frame.dlc = line[dlcPos] - '0';
    size_t expected = dlcPos + 1 + (frame.rtr ? 0 : frame.dlc * 2);
    // Some adapters append a four-digit timestamp; reject every other trailing payload.
    if (line.length() != expected && line.length() != expected + 4) return false;
    for (uint8_t i = 0; !frame.rtr && i < frame.dlc; ++i) {
        int hi = hexNibble(line[dlcPos + 1 + i * 2]);
        int lo = hexNibble(line[dlcPos + 2 + i * 2]);
        if (hi < 0 || lo < 0) return false;
        frame.data[i] = (hi << 4) | lo;
    }
    frame.timestampMs = millis();
    return validateFrame(frame);
}

String SlcanAdapter::encodeFrame(const CanFrame &frame) {
    if (!validateFrame(frame)) return "";
    char buffer[32];
    snprintf(buffer, sizeof(buffer), frame.extended ? "%c%08lX%u" : "%c%03lX%u",
             frame.rtr ? (frame.extended ? 'R' : 'r') : (frame.extended ? 'T' : 't'),
             static_cast<unsigned long>(frame.id), frame.dlc);
    String result(buffer);
    if (!frame.rtr) for (uint8_t i = 0; i < frame.dlc; ++i) {
        snprintf(buffer, sizeof(buffer), "%02X", frame.data[i]); result += buffer;
    }
    return result;
}

bool SlcanAdapter::poll(CanFrame &frame) {
    while (_serial.available()) {
        char c = static_cast<char>(_serial.read());
        if (c == '\a') { _errors++; _lastError = "Erreur SLCAN"; _line = ""; continue; }
        if (c == '\r' || c == '\n') {
            if (!_line.length()) continue;
            String complete = _line; _line = "";
            if (parseFrame(complete, frame)) { frame.tx = false; _rxFrames++; return true; }
            _errors++; _lastError = "Ligne SLCAN invalide";
        } else if (isPrintable(c)) {
            if (_line.length() < 40) _line += c;
            else { _line = ""; _errors++; _lastError = "Ligne SLCAN trop longue"; }
        }
    }
    return false;
}

bool SlcanAdapter::send(const CanFrame &frame) {
    if (!_connected) { _lastError = "CAN ferme"; return false; }
    if (_listenOnly) { _lastError = "Emission verrouillee"; return false; }
    String encoded = encodeFrame(frame);
    if (!encoded.length()) { _lastError = "Trame invalide"; return false; }
    bool ok = transact(encoded);
    if (ok) { _txFrames++; CanFrame sent = frame; sent.tx = true; sent.timestampMs = millis(); sharedActivity.update(sent); }
    return ok;
}

CanTransportStatus SlcanAdapter::status() const {
    CanTransportStatus s; s.connected = _connected; s.listenOnly = _listenOnly;
    s.txUnlocked = _connected && !_listenOnly; s.bitrate = _bitrate; s.rxFrames = _rxFrames;
    s.txFrames = _txFrames; s.errors = _errors; s.lastError = _lastError;
    s.busState = !_connected ? CanBusState::STOPPED : (_listenOnly ? CanBusState::PASSIVE : CanBusState::ACTIVE);
    return s;
}

namespace {
bool twaiTiming(uint32_t bitrate, twai_timing_config_t &out) {
    switch (bitrate) {
        // ESP32 rev. 2+ accepts this timing, although ESP-IDF hides the convenience
        // macro when the minimum silicon revision is not fixed at compile time.
        case 10000: out = {TWAI_CLK_SRC_DEFAULT, 200000, 0, 0, 15, 4, 3, 0, false}; break;
        case 20000: out = TWAI_TIMING_CONFIG_20KBITS(); break;
        case 50000: out = TWAI_TIMING_CONFIG_50KBITS(); break;
        case 100000: out = TWAI_TIMING_CONFIG_100KBITS(); break;
        case 125000: out = TWAI_TIMING_CONFIG_125KBITS(); break;
        case 250000: out = TWAI_TIMING_CONFIG_250KBITS(); break;
        case 500000: out = TWAI_TIMING_CONFIG_500KBITS(); break;
        case 800000: out = TWAI_TIMING_CONFIG_800KBITS(); break;
        case 1000000: out = TWAI_TIMING_CONFIG_1MBITS(); break;
        default: return false;
    }
    return true;
}
}

bool TwaiTransport::begin(uint32_t bitrate, int8_t rxPin, int8_t txPin, bool listenOnly) {
    end(); _status = {}; _status.listenOnly = listenOnly; _status.bitrate = bitrate;
    twai_timing_config_t timing{};
    if (!twaiTiming(bitrate, timing)) { _status.lastError = "Debit TWAI non supporte"; return false; }
    twai_general_config_t general = TWAI_GENERAL_CONFIG_DEFAULT(
        static_cast<gpio_num_t>(txPin), static_cast<gpio_num_t>(rxPin),
        listenOnly ? TWAI_MODE_LISTEN_ONLY : TWAI_MODE_NORMAL
    );
    general.rx_queue_len = 128; general.tx_queue_len = listenOnly ? 0 : 32;
    general.alerts_enabled = TWAI_ALERT_BUS_OFF | TWAI_ALERT_BUS_RECOVERED | TWAI_ALERT_ERR_PASS |
                             TWAI_ALERT_ABOVE_ERR_WARN | TWAI_ALERT_RX_QUEUE_FULL;
    twai_filter_config_t filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    esp_err_t result = twai_driver_install(&general, &timing, &filter);
    if (result != ESP_OK) { _status.lastError = "Installation TWAI: " + String(esp_err_to_name(result)); return false; }
    _installed = true;
    result = twai_start();
    if (result != ESP_OK) { _status.lastError = "Demarrage TWAI: " + String(esp_err_to_name(result)); end(); return false; }
    _status.connected = true; _status.txUnlocked = !listenOnly;
    _status.busState = listenOnly ? CanBusState::PASSIVE : CanBusState::ACTIVE;
    return true;
}

void TwaiTransport::end() {
    if (_installed) { twai_stop(); twai_driver_uninstall(); }
    _installed = false; _status.connected = false; _status.txUnlocked = false;
    _status.busState = CanBusState::STOPPED;
}

bool TwaiTransport::poll(CanFrame &frame) {
    if (!_installed) return false;
    uint32_t alerts = 0;
    if (twai_read_alerts(&alerts, 0) == ESP_OK && alerts) {
        if (alerts & TWAI_ALERT_BUS_OFF) { _status.busState = CanBusState::BUS_OFF; _status.errors++; _status.lastError = "CAN bus-off"; }
        if (alerts & TWAI_ALERT_BUS_RECOVERED) _status.busState = _status.listenOnly ? CanBusState::PASSIVE : CanBusState::ACTIVE;
        if (alerts & (TWAI_ALERT_ERR_PASS | TWAI_ALERT_ABOVE_ERR_WARN)) { _status.busState = CanBusState::WARNING; _status.errors++; }
        if (alerts & TWAI_ALERT_RX_QUEUE_FULL) _status.rxMissed++;
    }
    twai_message_t message{};
    if (twai_receive(&message, 0) != ESP_OK) return false;
    frame = {}; frame.id = message.identifier; frame.extended = message.extd;
    frame.rtr = message.rtr; frame.dlc = min<uint8_t>(message.data_length_code, 8);
    for (uint8_t i=0; !frame.rtr && i<frame.dlc; ++i) frame.data[i] = message.data[i];
    frame.timestampMs = millis(); _status.rxFrames++; return true;
}

bool TwaiTransport::send(const CanFrame &frame) {
    if (!_installed || _status.listenOnly || !_status.txUnlocked) { _status.lastError = "Emission verrouillee"; return false; }
    String error; if (!validateFrame(frame, &error)) { _status.lastError = error; return false; }
    twai_message_t message{}; message.identifier = frame.id; message.extd = frame.extended;
    message.rtr = frame.rtr; message.data_length_code = frame.dlc;
    for (uint8_t i=0; !frame.rtr && i<frame.dlc; ++i) message.data[i] = frame.data[i];
    if (twai_transmit(&message, pdMS_TO_TICKS(50)) != ESP_OK) { _status.errors++; _status.lastError = "File TX pleine"; return false; }
    _status.txFrames++; CanFrame sent=frame; sent.tx=true; sent.timestampMs=millis(); sharedActivity.update(sent); return true;
}

CanTransportStatus TwaiTransport::status() const {
    CanTransportStatus result = _status;
    if (_installed) { twai_status_info_t info{}; if (twai_get_status_info(&info)==ESP_OK) { result.errors += info.tx_error_counter + info.rx_error_counter; result.rxMissed += info.rx_missed_count + info.rx_overrun_count; } }
    return result;
}

bool validateFrame(const CanFrame &frame, String *error) {
    if (frame.dlc > 8) { setError(error, "DLC doit etre 0..8"); return false; }
    if ((!frame.extended && frame.id > 0x7FF) || (frame.extended && frame.id > 0x1FFFFFFF)) {
        setError(error, "ID CAN hors plage"); return false;
    }
    return true;
}

CanActivityTable::CanActivityTable() : _mutex(xSemaphoreCreateMutex()) {}
void CanActivityTable::clear() {
    if (_mutex) xSemaphoreTake(_mutex, portMAX_DELAY);
    _items.clear();
    if (_mutex) xSemaphoreGive(_mutex);
}
std::vector<CanIdStats> CanActivityTable::snapshot() const {
    if (_mutex) xSemaphoreTake(_mutex, portMAX_DELAY);
    std::vector<CanIdStats> copy = _items;
    if (_mutex) xSemaphoreGive(_mutex);
    return copy;
}
const CanIdStats *CanActivityTable::find(uint32_t id, bool ext) const {
    for (const auto &item : _items) if (item.id == id && item.extended == ext) return &item;
    return nullptr;
}
void CanActivityTable::update(const CanFrame &frame) {
    if (_mutex) xSemaphoreTake(_mutex, portMAX_DELAY);
    CanIdStats *item = nullptr;
    for (auto &candidate : _items) if (candidate.id == frame.id && candidate.extended == frame.extended) { item = &candidate; break; }
    if (!item) {
        if (_items.size() >= MAX_ACTIVE_IDS) { if (_mutex) xSemaphoreGive(_mutex); return; }
        _items.push_back({}); item = &_items.back(); item->id = frame.id; item->extended = frame.extended;
        item->firstSeenMs = frame.timestampMs;
    } else {
        uint8_t common = min(item->lastFrame.dlc, frame.dlc);
        for (uint8_t i = 0; i < common; ++i) if (item->lastFrame.data[i] != frame.data[i]) item->changedMask |= 1U << i;
        if (item->lastFrame.dlc != frame.dlc) item->changedMask |= 0xFF;
    }
    item->count++; item->lastSeenMs = frame.timestampMs; item->lastFrame = frame;
    uint32_t span = item->lastSeenMs - item->firstSeenMs;
    item->frequencyHz = span ? (item->count - 1) * 1000.0f / span : 0;
    if (_mutex) xSemaphoreGive(_mutex);
}

bool parseBitrate(const String &value, uint32_t &bitrate) {
    bitrate = value.toInt(); return SlcanAdapter::bitrateCode(bitrate) != 0;
}

bool filterAccepts(const CanFrame &frame, const std::vector<CanFilter> &inc, const std::vector<CanFilter> &exc) {
    auto matches = [&](const std::vector<CanFilter> &items) { for (const auto &f : items) if (f.id == frame.id && f.extended == frame.extended) return true; return false; };
    return (inc.empty() || matches(inc)) && !matches(exc);
}

bool loadFilters(FS &fs, std::vector<CanFilter> &inc, std::vector<CanFilter> &exc) {
    inc.clear(); exc.clear(); File file = fs.open("/Automotive/filters.json", FILE_READ); if (!file) return false;
    if (file.size() > 16384) { file.close(); return false; }
    JsonDocument doc; if (deserializeJson(doc, file)) { file.close(); return false; } file.close();
    auto read = [](JsonArrayConst array, std::vector<CanFilter> &out) { for (JsonObjectConst o : array) { if (out.size() >= MAX_ACTIVE_IDS) break; CanFilter f{o["id"] | 0U, o["extended"] | false}; if ((f.extended && f.id <= 0x1FFFFFFF) || (!f.extended && f.id <= 0x7FF)) out.push_back(f); } };
    read(doc["include"].as<JsonArrayConst>(), inc); read(doc["exclude"].as<JsonArrayConst>(), exc); return true;
}

bool saveFilters(FS &fs, const std::vector<CanFilter> &inc, const std::vector<CanFilter> &exc) {
    fs.mkdir("/Automotive"); JsonDocument doc;
    auto write = [&](const char *key, const std::vector<CanFilter> &items) { JsonArray a = doc[key].to<JsonArray>(); for (const auto &f : items) { JsonObject o = a.add<JsonObject>(); o["id"] = f.id; o["extended"] = f.extended; } };
    write("include", inc); write("exclude", exc); File f = fs.open("/Automotive/filters.json.tmp", FILE_WRITE); if (!f) return false;
    bool ok = serializeJson(doc, f) > 0; f.close(); if (!ok) return false;
    fs.remove("/Automotive/filters.json"); return fs.rename("/Automotive/filters.json.tmp", "/Automotive/filters.json");
}

String scenarioPath(const String &name) { return safeName(name) ? "/Automotive/scenarios/" + name + ".json" : ""; }

bool loadScenario(FS &fs, const String &path, CanScenario &scenario, String *error) {
    File file = fs.open(path, FILE_READ); if (!file) { setError(error, "scenario introuvable"); return false; }
    if (file.size() > 49152) { file.close(); setError(error, "scenario trop volumineux"); return false; }
    JsonDocument doc; DeserializationError de = deserializeJson(doc, file); file.close(); if (de) { setError(error, de.c_str()); return false; }
    scenario = {}; scenario.name = doc["name"] | ""; scenario.bitrate = doc["bitrate"] | 0U;
    if (!safeName(scenario.name) || !SlcanAdapter::bitrateCode(scenario.bitrate)) { setError(error, "nom/debit invalide"); return false; }
    JsonArrayConst steps = doc["steps"].as<JsonArrayConst>(); if (!steps.size() || steps.size() > 256) { setError(error, "1..256 etapes requises"); return false; }
    for (JsonObjectConst o : steps) {
        CanScenarioStep step; step.delayMs = o["delayMs"] | 0U; step.repetitions = o["repetitions"] | 1;
        if (step.delayMs > 3600000 || step.repetitions < 1 || step.repetitions > 10000 || !jsonToFrame(o["frame"], step.frame, error)) return false;
        scenario.steps.push_back(step);
    }
    return true;
}

bool saveScenarioAtomic(FS &fs, const CanScenario &scenario, String *error) {
    String path = scenarioPath(scenario.name); if (!path.length() || !SlcanAdapter::bitrateCode(scenario.bitrate) || scenario.steps.empty() || scenario.steps.size() > 256) { setError(error, "scenario invalide"); return false; }
    fs.mkdir("/Automotive"); fs.mkdir("/Automotive/scenarios"); JsonDocument doc; doc["name"] = scenario.name; doc["bitrate"] = scenario.bitrate; JsonArray steps = doc["steps"].to<JsonArray>();
    for (const auto &s : scenario.steps) {
        if (s.delayMs > 3600000 || s.repetitions < 1 || s.repetitions > 10000 || !validateFrame(s.frame, error)) return false;
        JsonObject o = steps.add<JsonObject>(); o["delayMs"] = s.delayMs; o["repetitions"] = s.repetitions; JsonObject f = o["frame"].to<JsonObject>(); f["id"] = s.frame.id; f["extended"] = s.frame.extended; f["rtr"] = s.frame.rtr; f["dlc"] = s.frame.dlc; JsonArray d = f["data"].to<JsonArray>(); if (!s.frame.rtr) for (uint8_t i=0;i<s.frame.dlc;i++) d.add(s.frame.data[i]);
    }
    String tmp = path + ".tmp", backup = path + ".bak";
    fs.remove(tmp); fs.remove(backup);
    File file = fs.open(tmp, FILE_WRITE); if (!file) { setError(error, "ecriture impossible"); return false; }
    bool ok = serializeJson(doc, file) > 0; file.flush(); file.close();
    if (!ok) { fs.remove(tmp); setError(error, "serialization impossible"); return false; }
    bool hadOriginal = fs.exists(path);
    if (hadOriginal && !fs.rename(path, backup)) { fs.remove(tmp); setError(error, "sauvegarde impossible"); return false; }
    if (!fs.rename(tmp, path)) {
        if (hadOriginal) fs.rename(backup, path);
        setError(error, "renommage impossible"); return false;
    }
    fs.remove(backup); return true;
}

bool deleteScenario(FS &fs, const String &name) { String p = scenarioPath(name); return p.length() && fs.exists(p) && fs.remove(p); }

String frameToJson(const CanFrame &f) {
    JsonDocument doc; doc["id"] = f.id; doc["extended"] = f.extended; doc["rtr"] = f.rtr; doc["tx"] = f.tx; doc["dlc"] = f.dlc; doc["timestampMs"] = f.timestampMs; JsonArray data = doc["data"].to<JsonArray>(); if (!f.rtr) for (uint8_t i=0;i<f.dlc;i++) data.add(f.data[i]); String out; serializeJson(doc, out); return out;
}

SlcanAdapter &adapter() { return sharedAdapter; }
TwaiTransport &twaiAdapter() { return sharedTwai; }

const char *backendName(CanBackend value) { return value == CanBackend::TWAI ? "TWAI" : "SLCAN"; }
CanBackend backend() { return selectedBackend; }

bool setBackend(CanBackend value) {
    stopTransport(); selectedBackend = value;
    Preferences prefs; if (!prefs.begin("sentinel-can", false)) return false;
    prefs.putUChar("backend", static_cast<uint8_t>(value)); prefs.end(); return true;
}

CanTransport &transport() { return selectedBackend == CanBackend::TWAI ? static_cast<CanTransport &>(sharedTwai) : static_cast<CanTransport &>(sharedAdapter); }
CanTransportStatus transportStatus() { CanTransportStatus s=transport().status(); s.txUnlocked=txUnlocked && s.connected && !s.listenOnly; return s; }

bool startTransport(uint32_t bitrate, int8_t rxPin, int8_t txPin) {
    static bool loaded = false;
    if (!loaded) { Preferences prefs; if (prefs.begin("sentinel-can", true)) { selectedBackend = static_cast<CanBackend>(min<uint8_t>(prefs.getUChar("backend", 0), 1)); prefs.end(); } loaded=true; }
    stopTransport();
    if (!acquireGrove(GroveOwner::CAN)) return false;
    activeBitrate=bitrate; activeRxPin=rxPin; activeTxPin=txPin; txUnlocked=false;
    if (transport().begin(bitrate, rxPin, txPin, true)) return true;
    releaseGrove(GroveOwner::CAN); return false;
}

void stopTransport() { sharedTwai.end(); sharedAdapter.end(); txUnlocked=false; releaseGrove(GroveOwner::CAN); }

bool unlockTransmission() {
    if (!activeBitrate || activeRxPin < 0 || activeTxPin < 0) return false;
    transport().end();
    if (!transport().begin(activeBitrate, activeRxPin, activeTxPin, false)) { txUnlocked=false; return false; }
    txUnlocked=true; return true;
}

void lockTransmission() {
    if (!activeBitrate) { txUnlocked=false; return; }
    transport().end(); transport().begin(activeBitrate, activeRxPin, activeTxPin, true); txUnlocked=false;
}

bool transmissionUnlocked() { return txUnlocked; }
CanActivityTable &activity() { return sharedActivity; }
} // namespace Automotive
