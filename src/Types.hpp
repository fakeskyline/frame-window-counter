#pragma once

#include <Geode/Geode.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace fwc {

enum class ActionType : uint8_t { Press = 0, Release = 1 };

enum class WindowClass : uint8_t { Unknown, FW1, FW2, FW3_4, FW5Plus };

struct ActionData {
    uint64_t frame = 0;
    double timeSeconds = 0.0;
    double window = 0.0; // 0 = unknown; GDR/SLC do not encode FW size.
    bool player2 = false;
    ActionType type = ActionType::Press;
    uint32_t inputIndex = 0;
    uint32_t inputsPerFrame = 1;
    cocos2d::CCPoint screenPosition = cocos2d::CCPointZero;
};

inline WindowClass classifyWindow(double window) {
    if (window <= 0.0) return WindowClass::Unknown;
    if (window <= 1.0) return WindowClass::FW1;
    if (window <= 2.0) return WindowClass::FW2;
    if (window <= 4.0) return WindowClass::FW3_4;
    return WindowClass::FW5Plus;
}

inline const char* windowLabel(WindowClass c) {
    switch (c) {
        case WindowClass::FW1: return "FW1";
        case WindowClass::FW2: return "FW2";
        case WindowClass::FW3_4: return "FW3-4";
        case WindowClass::FW5Plus: return "FW5+";
        default: return "?";
    }
}

inline cocos2d::ccColor4F windowColor(double window) {
    switch (classifyWindow(window)) {
        case WindowClass::FW1: return {1.f, 0.12f, 0.12f, 1.f};
        case WindowClass::FW2: return {1.f, 0.56f, 0.05f, 1.f};
        case WindowClass::FW3_4: return {1.f, 0.88f, 0.10f, 1.f};
        case WindowClass::FW5Plus: return {0.18f, 0.9f, 0.28f, 1.f};
        default: return {0.75f, 0.75f, 0.75f, 1.f};
    }
}

struct Stats {
    size_t total = 0;
    size_t fw1 = 0;
    size_t fw2 = 0;
    size_t fw3 = 0;
    size_t fw4 = 0;
    size_t fw5_6 = 0;
    size_t fw7_8 = 0;
    size_t fw9_10 = 0;
    size_t unknown = 0;
};

} // namespace fwc
