#pragma once

#include <Arduino.h>
#include <FS.h>
#include <HardwareSerial.h>
#include <array>
#include <vector>

namespace Automotive {

constexpr size_t MAX_ACTIVE_IDS = 128;
constexpr uint32_t SLCAN_BAUD = 115200;

struct CanFrame {
    uint32_t id = 0;
    bool extended = false;
    bool rtr = false;
    bool tx = false;
    uint8_t dlc = 0;
    std::array<uint8_t, 8> data{};
    uint32_t timestampMs = 0;
};

struct CanIdStats {
    uint32_t id = 0;
    bool extended = false;
    uint32_t count = 0;
    uint32_t firstSeenMs = 0;
    uint32_t lastSeenMs = 0;
    float frequencyHz = 0;
    uint8_t changedMask = 0;
    CanFrame lastFrame;
};

struct CanScenarioStep {
    CanFrame frame;
    uint32_t delayMs = 0;
    uint16_t repetitions = 1;
};

struct CanScenario {
    String name;
    uint32_t bitrate = 500000;
    std::vector<CanScenarioStep> steps;
};

struct CanFilter {
    uint32_t id = 0;
    bool extended = false;
};

class SlcanAdapter {
public:
    explicit SlcanAdapter(HardwareSerial &serial);
    bool begin(uint32_t bitrate, int8_t rxPin, int8_t txPin);
    void end();
    bool poll(CanFrame &frame);
    bool send(const CanFrame &frame);
    bool connected() const { return _connected; }
    uint32_t bitrate() const { return _bitrate; }
    uint32_t errors() const { return _errors; }
    String lastError() const { return _lastError; }

    static bool parseFrame(const String &line, CanFrame &frame);
    static String encodeFrame(const CanFrame &frame);
    static char bitrateCode(uint32_t bitrate);

private:
    bool transact(const String &command, uint32_t timeoutMs = 250);
    HardwareSerial &_serial;
    String _line;
    bool _connected = false;
    uint32_t _bitrate = 0;
    uint32_t _errors = 0;
    String _lastError;
};

class CanActivityTable {
public:
    void clear();
    void update(const CanFrame &frame);
    const std::vector<CanIdStats> &items() const { return _items; }
    const CanIdStats *find(uint32_t id, bool extended) const;
private:
    std::vector<CanIdStats> _items;
};

bool validateFrame(const CanFrame &frame, String *error = nullptr);
bool parseBitrate(const String &value, uint32_t &bitrate);
bool loadFilters(FS &fs, std::vector<CanFilter> &include, std::vector<CanFilter> &exclude);
bool saveFilters(FS &fs, const std::vector<CanFilter> &include, const std::vector<CanFilter> &exclude);
bool filterAccepts(const CanFrame &frame, const std::vector<CanFilter> &include,
                   const std::vector<CanFilter> &exclude);
bool loadScenario(FS &fs, const String &path, CanScenario &scenario, String *error = nullptr);
bool saveScenarioAtomic(FS &fs, const CanScenario &scenario, String *error = nullptr);
bool deleteScenario(FS &fs, const String &name);
String scenarioPath(const String &name);
String frameToJson(const CanFrame &frame);

SlcanAdapter &adapter();
CanActivityTable &activity();

} // namespace Automotive
