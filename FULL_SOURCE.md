# Frame Window Counter — полный листинг исходников

Ниже приведены содержимое всех файлов проекта на момент сборки архива.

## `.github/workflows/multi-platform.yml`

```yaml
name: Build Geode Mod

on:
  workflow_dispatch:
  push:
    branches:
      - "**"

jobs:
  build:
    strategy:
      fail-fast: false
      matrix:
        config:
          - name: Windows
            os: windows-latest
            target: Win64
          - name: macOS
            os: macos-latest
            target: MacOS
          - name: iOS
            os: macos-latest
            target: iOS
          - name: Android32
            os: ubuntu-latest
            target: Android32
          - name: Android64
            os: ubuntu-latest
            target: Android64
    name: ${{ matrix.config.name }}
    runs-on: ${{ matrix.config.os }}
    steps:
      - uses: actions/checkout@v7
      - name: Build the mod
        uses: geode-sdk/build-geode-mod@main
        with:
          combine: true
          target: ${{ matrix.config.target }}

  package:
    name: Package builds
    runs-on: ubuntu-latest
    needs: [build]
    steps:
      - uses: geode-sdk/build-geode-mod/combine@main
        id: build
      - uses: actions/upload-artifact@v7
        with:
          name: frame-window-counter
          path: ${{ steps.build.outputs.build-output }}
```

## `.gitignore`

```text
build/
out/
.vscode/
.idea/
*.user
*.log
```

## `CMakeLists.txt`

```text
cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
elseif(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
project(FrameWindowCounter VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.hpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)
include($ENV{GEODE_SDK}/cmake/CPM.cmake)

# GDR2 + legacy GDR import.
CPMAddPackage("gh:maxnut/GDReplayFormat#2a0ca3cff9b1d16863037a464da5fe27c28a5823")
CPMAddPackage(
    NAME GDRconverter
    GITHUB_REPOSITORY maxnut/GDR-converter
    GIT_TAG main
    EXCLUDE_FROM_ALL YES
    OPTIONS "BUILD_TESTING OFF"
)

# Silicate SLC v2/v3 reader.
FetchContent_Declare(
    slc_repo
    GIT_REPOSITORY https://github.com/silicate-bot/slc.git
    GIT_TAG main
)
FetchContent_MakeAvailable(slc_repo)

target_link_libraries(${PROJECT_NAME} PRIVATE libGDR GDRconverter)
target_include_directories(${PROJECT_NAME} PRIVATE ${slc_repo_SOURCE_DIR}/include)

setup_geode_mod(${PROJECT_NAME})
```

## `LICENSE`

```text
MIT License

Copyright (c) 2026 SudoSuGD

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## `README.md`

```markdown
# Frame Window Counter

A compact, cross-platform Geode mod for Geometry Dash 2.2081 that:

- captures player press/release events through `PlayerObject` hooks;
- timestamps them on a 240 Hz logical clock;
- displays animated markers and a live HUD;
- imports `.gdr`, `.gdr2`, `.slc`, `.json`, and `.fwc`;
- supports manual FW stamping;
- calculates an approximate `σ/s` precision for a 24-hour target;
- keeps the HUD under `OverlayManager` across scene changes;
- builds with Geode's official multi-platform GitHub Action.

## The key technical distinction

The input hook tells us **when** an input happened. It does not reveal the true acceptance interval of that timing. Frame-window size must therefore come from measured data, `.fwc`, NaNDL JSON, or manual assignment. GDR/SLC imports intentionally preserve the FW as unknown instead of assuming FW1.

## Build locally

```bash
geode build
```

## GitHub Actions

The workflow builds Windows, macOS, iOS, Android32 and Android64 independently, then combines the artifacts into one `.geode`.

## Termux upload

See `TERMUX.md`.
```

## `TERMUX.md`

```markdown
# Termux → GitHub → Geode build

> Android is enough to upload the source. The actual Geode compilation happens on GitHub Actions.

## 1. Install Termux

Use the F-Droid build of Termux. Then:

```bash
pkg update -y
pkg install git gh -y
```

## 2. Authenticate GitHub

```bash
gh auth login
```

Choose GitHub.com, HTTPS, and complete the browser/device-code login.

## 3. Create the repository

On GitHub create an empty repository named `frame-window-counter`.

In Termux, enter your project folder:

```bash
cd ~/frame-window-counter
```

Then:

```bash
git init
git add .
git commit -m "Initial commit"
git branch -M main
git remote add origin https://github.com/ВАШ_ЮЗЕРНЕЙМ/frame-window-counter.git
git push -u origin main
```

## 4. Start the build

Open the repository on GitHub → **Actions** → **Build Geode Mod** → **Run workflow**.

The same workflow also runs automatically after a push.

## 5. Get the `.geode`

Wait for all platform jobs to pass. Open the `package` job and download the artifact named **frame-window-counter**. It contains the combined `.geode` package.

## 6. Install

Copy the `.geode` into the Geometry Dash Geode `mods` directory on the device/PC where Geometry Dash is installed, then launch the game.

## 7. Manual frame-window workflow

For live captures:

1. Start a level.
2. The mod records press/release events and assigns a 240 Hz frame/time coordinate.
3. Set **Manual Window** to `1..10` in Geode settings.
4. Press **Manual Stamp Key** after selecting the measured FW for the latest unknown action.
5. Imported `.gdr/.gdr2/.slc` files also start with unknown FW values; this is expected.

For bulk work, prepare a NaNDL JSON or `.fwc` file with actual `frameWindow` values and set **Import Path**, then press **Import Key**.
```

## `about.md`

```markdown
# Frame Window Counter

A Geode utility for logging player inputs, visualizing known frame windows, importing common replay formats, and estimating an approximate **σ/s precision** needed for a 24-hour completion target.

## Important limitation

A `.gdr/.gdr2/.slc` file stores input timing, but not the measured frame-window tolerance itself. This mod therefore imports those files with **unknown FW** values. Assign the measured FW manually with **Manual Window + Manual Stamp**, or use a `.fwc` / NaNDL JSON file that already contains `frameWindow` values.

This is intentional: the mod does not invent a timing window from a replay timestamp.

## Marker colors

- FW1: red
- FW2: orange
- FW3-4: yellow
- FW5+: green
- Unknown: gray

## Precision

The calculator uses a Gaussian/erfc-style pass-probability model inspired by the public NaNDL precision calculator. It is an approximation and should not be treated as an official Geometry Dash difficulty score.
```

## `mod.json`

```json
{
  "geode": "5.10.1",
  "gd": {
    "win": "2.2081",
    "android": "2.2081",
    "mac": "2.2081",
    "ios": "2.2081"
  },
  "version": "v1.0.0",
  "id": "sudosugd.frame-window-counter",
  "name": "Frame Window Counter",
  "developer": "SudoSuGD",
  "description": "Frame-window visualization, macro analysis and approximate sigma-per-second precision metrics.",
  "tags": ["utility", "customization"],
  "links": {
    "source": "https://github.com/ВАШ_ЮЗЕРНЕЙМ/frame-window-counter"
  },
  "settings": {
    "enabled": {
      "type": "bool",
      "name": "Enabled",
      "description": "Enable the Frame Window Counter overlay and hooks.",
      "default": true
    },
    "hud-x": {
      "type": "int",
      "name": "HUD X",
      "description": "Horizontal HUD position in screen points.",
      "default": 10,
      "min": 0,
      "max": 2000,
      "control": {"input": true, "arrows": true}
    },
    "hud-y": {
      "type": "int",
      "name": "HUD Y",
      "description": "Vertical HUD position in screen points.",
      "default": 10,
      "min": 0,
      "max": 2000,
      "control": {"input": true, "arrows": true}
    },
    "font-size": {
      "type": "float",
      "name": "Font Size",
      "description": "HUD label scale.",
      "default": 0.5,
      "min": 0.2,
      "max": 1.5,
      "control": {"slider": true, "slider-step": 0.05, "input": true}
    },
    "show-markers": {
      "type": "bool",
      "name": "Show Markers",
      "description": "Show animated action markers.",
      "default": true
    },
    "sound-fw1": {
      "type": "string",
      "name": "FW1 Sound",
      "description": "Optional absolute or game-search-path sound effect for FW1.",
      "default": ""
    },
    "sound-fw2": {
      "type": "string",
      "name": "FW2 Sound",
      "description": "Optional sound effect for FW2.",
      "default": ""
    },
    "sound-fw3": {
      "type": "string",
      "name": "FW3+ Sound",
      "description": "Optional sound effect for FW3 and above.",
      "default": ""
    },
    "manual-window": {
      "type": "int",
      "name": "Manual Window",
      "description": "Window assigned when Manual Stamp is pressed.",
      "default": 1,
      "min": 1,
      "max": 10,
      "control": {"slider": true, "input": true, "slider-step": 1}
    },
    "manual-stamp": {
      "type": "keybind",
      "name": "Manual Stamp Key",
      "description": "Assign Manual Window to the latest captured action.",
      "default": "F8"
    },
    "reset-key": {
      "type": "keybind",
      "name": "Reset Key",
      "description": "Clear the current run statistics.",
      "default": "F7"
    },
    "import-path": {
      "type": "string",
      "name": "Import Path",
      "description": "Path to a .gdr/.gdr2/.slc/.json/.fwc file. The Manual Import key loads this file.",
      "default": ""
    },
    "import-key": {
      "type": "keybind",
      "name": "Import Key",
      "description": "Load the file specified by Import Path.",
      "default": "F9"
    }
  }
}
```

## `src/FrameWindowCounter.cpp`

```cpp
#include "FrameWindowCounter.hpp"
#include "State.hpp"
#include "Precision.hpp"
#include "Types.hpp"
#include <Geode/Geode.hpp>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace fwc {

FrameWindowCounter* FrameWindowCounter::create() {
    auto ret = new FrameWindowCounter();
    if (ret && ret->init()) { ret->autorelease(); return ret; }
    delete ret;
    return nullptr;
}

bool FrameWindowCounter::init() {
    if (!CCNode::init()) return false;
    setID("frame-window-counter");
    m_markers = CCNode::create();
    m_graph = CCDrawNode::create();
    m_hud = CCLabelBMFont::create("Frame Window Counter", "bigFont.fnt");
    if (!m_markers || !m_graph || !m_hud) return false;

    addChild(m_graph, 0);
    addChild(m_markers, 1);
    addChild(m_hud, 2);
    m_hud->setAnchorPoint({0.f, 1.f});
    scheduleUpdate();

    addEventListener(
        KeybindSettingPressedEventV3(Mod::get(), "manual-stamp"),
        [](Keybind const&, bool down, bool repeat, double) {
            if (down && !repeat) {
                auto value = Mod::get()->getSettingValue<int64_t>("manual-window");
                if (State::get().stampLatest(static_cast<double>(value))) {
                    auto key = value <= 1 ? "sound-fw1" : (value == 2 ? "sound-fw2" : "sound-fw3");
                    auto sound = Mod::get()->getSettingValue<std::string>(key);
                    if (!sound.empty()) FMODAudioEngine::sharedEngine()->playEffect(sound.c_str());
                    if (auto hud = FrameWindowCounter::get()) hud->refresh();
                }
            }
        }
    );
    addEventListener(
        KeybindSettingPressedEventV3(Mod::get(), "reset-key"),
        [](Keybind const&, bool down, bool repeat, double) {
            if (down && !repeat) State::get().reset();
        }
    );
    return true;
}

void FrameWindowCounter::update(float dt) {
    if (!Mod::get()->getSettingValue<bool>("enabled")) return;
    m_elapsed += std::max(0.f, dt);
    uint64_t frame = static_cast<uint64_t>(std::floor(m_elapsed * 240.0 + 0.5));
    State::get().setLiveFrame(frame);
    State::get().setLiveTime(m_elapsed);
    m_refreshAccumulator += dt;
    if (m_refreshAccumulator >= 0.05) {
        m_refreshAccumulator = 0.0;
        refresh();
    }
}

void FrameWindowCounter::refresh() {
    auto s = State::get().stats();
    auto actions = State::get().snapshot();
    auto p = calculatePrecision(actions, State::get().fps());

    m_hud->setScale(static_cast<float>(Mod::get()->getSettingValue<double>("font-size")));
    m_hud->setPosition(
        static_cast<float>(Mod::get()->getSettingValue<int64_t>("hud-x")),
        CCDirector::get()->getWinSize().height - static_cast<float>(Mod::get()->getSettingValue<int64_t>("hud-y"))
    );

    auto text = fmt::format(
        "FW1 {:>3}   FW2 {:>3}\nFW3 {:>3}   FW4 {:>3}\nFW5-6 {:>3} FW7-8 {:>3}\nFW9-10 {:>3}\nTOTAL {:>4}  UNKNOWN {:>3}\nσ/s {:>8.2f}   RMS {:>6.2f}ms",
        s.fw1, s.fw2, s.fw3, s.fw4, s.fw5_6, s.fw7_8, s.fw9_10, s.total, s.unknown,
        p.sigmaPerSecond, p.rmsWindowMs
    );
    m_hud->setString(text.c_str());

    m_graph->clear();
    auto win = CCDirector::get()->getWinSize();
    float gx = 10.f;
    float gy = 105.f;
    float gw = std::min(240.f, win.width - 20.f);
    float gh = 32.f;
    size_t start = actions.size() > 60 ? actions.size() - 60 : 0;
    for (size_t i = start; i < actions.size(); ++i) {
        if (actions[i].window <= 0.0) continue;
        float x = gx + static_cast<float>(i - start) / 59.f * gw;
        float y = gy + std::min(gh, static_cast<float>(actions[i].window) / 10.f * gh);
        float nx = x + gw / 59.f;
        float ny = gy + std::min(gh, static_cast<float>(actions[i].window) / 10.f * gh);
        auto c = windowColor(actions[i].window);
        m_graph->drawSegment({x, gy}, {nx, ny}, 1.2f, c);
    }
}

void FrameWindowCounter::mark(CCPoint pos, double window) {
    if (!Mod::get()->getSettingValue<bool>("show-markers")) return;
    auto dot = CCDrawNode::create();
    dot->setPosition(pos);
    dot->drawDot(CCPointZero, 5.f, windowColor(window));
    m_markers->addChild(dot);
    dot->setOpacity(255);
    dot->runAction(CCSpawn::create(
        CCFadeOut::create(0.45f),
        CCScaleTo::create(0.45f, 1.7f),
        nullptr
    ));
    dot->runAction(CCSequence::create(CCDelayTime::create(0.46f), CCCallFuncN::create(dot, [](CCNode* n){ n->removeFromParent(); }), nullptr));
}

static FrameWindowCounter* s_hud = nullptr;

FrameWindowCounter* getHUD() { return s_hud; }

void ensureHUD() {
    if (!Mod::get()->getSettingValue<bool>("enabled")) return;
    auto overlay = OverlayManager::get();
    if (!overlay) return;
    if (!s_hud) {
        s_hud = FrameWindowCounter::create();
        if (s_hud) overlay->addChild(s_hud, 99999);
    } else if (!s_hud->getParent()) {
        overlay->addChild(s_hud, 99999);
    }
}

void removeHUD() {
    if (s_hud) {
        s_hud->removeFromParentAndCleanup(true);
        s_hud = nullptr;
    }
}

} // namespace fwc
```

## `src/FrameWindowCounter.hpp`

```cpp
#pragma once
#include <Geode/Geode.hpp>

namespace fwc {

class FrameWindowCounter : public cocos2d::CCNode {
public:
    static FrameWindowCounter* create();
    bool init();
    void update(float dt) override;
    void refresh();
    void mark(cocos2d::CCPoint screenPosition, double window);

private:
    cocos2d::CCLabelBMFont* m_hud = nullptr;
    cocos2d::CCDrawNode* m_graph = nullptr;
    cocos2d::CCNode* m_markers = nullptr;
    double m_elapsed = 0.0;
    double m_refreshAccumulator = 0.0;
};

FrameWindowCounter* getHUD();
void ensureHUD();
void removeHUD();

} // namespace fwc
```

## `src/Hooks.cpp`

```cpp
#include <Geode/Geode.hpp>
#include "FrameWindowCounter.hpp"
#include "State.hpp"
#include "MacroParser.hpp"

using namespace geode::prelude;
using namespace fwc;

namespace {
static void recordInput(PlayerObject* self, PlayerButton button, ActionType type) {
    if (!Mod::get()->getSettingValue<bool>("enabled")) return;
    auto* hud = getHUD();
    if (!hud) return;

    auto* parent = self->getParent();
    auto screen = parent ? parent->convertToWorldSpace(self->getPosition()) : self->getPosition();
    ActionData a;
    a.frame = State::get().liveFrame();
    a.timeSeconds = State::get().durationSeconds();
    a.window = 0.0;
    a.player2 = false;
    a.type = type;
    a.screenPosition = screen;
    State::get().add(a);
    hud->mark(screen, 0.0);
}

}

class $modify(FWCPlayerObject, PlayerObject) {
    bool pushButton(PlayerButton button) {
        bool ret = PlayerObject::pushButton(button);
        recordInput(this, button, ActionType::Press);
        return ret;
    }

    bool releaseButton(PlayerButton button) {
        bool ret = PlayerObject::releaseButton(button);
        recordInput(this, button, ActionType::Release);
        return ret;
    }
};

class $modify(FWCPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        State::get().reset();
        ensureHUD();
        return true;
    }

    void resetLevel() {
        State::get().reset();
        PlayLayer::resetLevel();
    }
};

$on_mod(Loaded) {
    auto node = CCNode::create();
    node->addEventListener(
        KeybindSettingPressedEventV3(Mod::get(), "import-key"),
        [](Keybind const&, bool down, bool repeat, double) {
            if (!down || repeat) return;
            auto raw = Mod::get()->getSettingValue<std::string>("import-path");
            if (raw.empty()) return;
            auto result = MacroParser::parse(raw);
            if (result.isErr()) {
                log::warn("Frame Window Counter import failed: {}", result.unwrapErr());
                return;
            }
            auto parsed = result.unwrap();
            State::get().importActions(std::move(parsed.actions), parsed.fps);
            if (getHUD()) getHUD()->refresh();
        }
    );
    OverlayManager::get()->addChild(node);
}
```

## `src/MacroParser.cpp`

```cpp
#include "MacroParser.hpp"
#include <Geode/Geode.hpp>
#include <gdr/gdr.hpp>
#include <gdr_convert.hpp>
#include <slc/slc.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cctype>

using namespace geode::prelude;

namespace {
struct Replay : gdr::Replay<Replay, gdr::Input<>> {
    Replay() : gdr::Replay<Replay, gdr::Input<>>("FrameWindowCounter", 1) {}
};

static fwc::ActionData makeAction(uint64_t frame, bool p2, bool down, double window, uint32_t index, double fps) {
    fwc::ActionData a;
    a.frame = frame;
    a.timeSeconds = frame / (fps > 0.0 ? fps : 240.0);
    a.window = window;
    a.player2 = p2;
    a.type = down ? fwc::ActionType::Press : fwc::ActionType::Release;
    a.inputIndex = index;
    return a;
}
}

namespace fwc::MacroParser {

geode::Result<Result> parse(std::filesystem::path const& path) {
    try {
        auto ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        Result out;

        if (ext == ".fwc") {
            std::ifstream f(path, std::ios::binary);
            if (!f) return Err("Cannot open .fwc");
            char magic[5]{};
            f.read(magic, 4);
            if (std::string(magic, 4) != "FWC2" && std::string(magic, 4) != "FWCB")
                return Err("Unsupported FWC signature");
            f.read(reinterpret_cast<char*>(&out.fps), sizeof(double));
            uint32_t count = 0;
            f.read(reinterpret_cast<char*>(&count), sizeof(uint32_t));
            for (uint32_t i = 0; i < count; ++i) {
                int32_t frame = 0, ifCount = 1;
                double fw = 1.0;
                uint8_t flags = 0;
                f.read(reinterpret_cast<char*>(&frame), sizeof(frame));
                if (std::string(magic, 4) == "FWCB") {
                    int32_t legacy = 1;
                    f.read(reinterpret_cast<char*>(&legacy), sizeof(legacy));
                    fw = legacy;
                } else {
                    f.read(reinterpret_cast<char*>(&fw), sizeof(fw));
                }
                f.read(reinterpret_cast<char*>(&flags), sizeof(flags));
                if (std::string(magic, 4) == "FWC2") f.read(reinterpret_cast<char*>(&ifCount), sizeof(ifCount));
                auto a = makeAction(static_cast<uint64_t>(std::max<int32_t>(0, frame)), (flags & 2) != 0, true, fw, i + 1, out.fps);
                a.inputsPerFrame = std::max(1, ifCount);
                out.actions.push_back(a);
            }
            return Ok(out);
        }

        if (ext == ".json") {
            std::ifstream f(path);
            if (!f) return Err("Cannot open .json");
            std::stringstream ss; ss << f.rdbuf();
            auto parsed = matjson::parse(ss.str());
            if (parsed.isErr()) return Err("Invalid JSON");
            auto root = parsed.unwrap();
            if (root.contains("settings")) out.fps = root["settings"]["gameFps"].asDouble().unwrapOr(240.0);
            if (!root.contains("frameWindows") || !root["frameWindows"].isArray()) return Err("No frameWindows array");
            auto arr = root["frameWindows"].asArray().unwrap();
            uint32_t i = 0;
            for (auto const& item : arr) {
                double time = item["timePosition"].asDouble().unwrapOr(0.0);
                double fw = item["frameWindow"].asDouble().unwrapOr(1.0);
                bool p2 = item["isPlayer2"].asBool().unwrapOr(false);
                auto a = makeAction(static_cast<uint64_t>(std::max(0.0, std::floor(time))), p2, true, fw, ++i, out.fps);
                a.timeSeconds = time / out.fps;
                a.inputsPerFrame = std::max(1, item["if"].asInt().unwrapOr(item["swift"].asInt().unwrapOr(0) + 1));
                out.actions.push_back(a);
            }
            return Ok(out);
        }

        if (ext == ".gdr2" || ext == ".gdr") {
            if (ext == ".gdr2") {
                auto r = Replay::importData(path);
                if (r.isErr()) return Err("Failed to parse .gdr2: " + r.unwrapErr());
                auto replay = r.unwrap();
                out.fps = replay.framerate > 0.0 ? replay.framerate : 240.0;
                uint32_t i = 0;
                for (auto const& input : replay.inputs) {
                    // GDR contains input timing, not frame-window size.
                    out.actions.push_back(makeAction(input.frame, input.player2, input.down, 0.0, ++i, out.fps));
                }
                return Ok(out);
            }
            std::ifstream f(path, std::ios::binary);
            if (!f) return Err("Cannot open .gdr");
            f.seekg(0, std::ios::end);
            size_t size = static_cast<size_t>(f.tellg());
            f.seekg(0, std::ios::beg);
            std::vector<uint8_t> bytes(size);
            f.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size));
            auto r = gdr::convert<Replay, gdr::Input<>>(std::span<uint8_t>(bytes));
            if (r.isErr()) return Err("Failed to convert legacy .gdr");
            auto replay = r.unwrap();
            out.fps = replay.framerate > 0.0 ? replay.framerate : 240.0;
            uint32_t i = 0;
            for (auto const& input : replay.inputs)
                out.actions.push_back(makeAction(input.frame, input.player2, input.down, 0.0, ++i, out.fps));
            return Ok(out);
        }

        if (ext == ".slc") {
            std::ifstream f(path, std::ios::binary);
            if (!f) return Err("Cannot open .slc");
            auto r = slc::Replay<>::read(f);
            if (!r) return Err("Failed to parse .slc");
            auto replay = r.value();
            out.fps = replay.m_meta.m_tps > 0.0 ? replay.m_meta.m_tps : 240.0;
            uint32_t i = 0;
            for (auto const& atom : replay.m_atoms.m_atoms) {
                if (auto const* actions = std::get_if<slc::ActionAtom>(&atom)) {
                    for (auto const& action : actions->m_actions) {
                        if (!action.isPlayer()) continue;
                        out.actions.push_back(makeAction(action.m_frame, action.m_player2, action.m_holding, 0.0, ++i, out.fps));
                    }
                }
            }
            return Ok(out);
        }

        return Err("Unsupported macro extension");
    } catch (std::exception const& e) {
        return Err(std::string("Parser exception: ") + e.what());
    }
}

void exportFWC2(std::filesystem::path const& path, std::vector<ActionData> const& actions, double fps) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return;
    f.write("FWC2", 4);
    f.write(reinterpret_cast<char const*>(&fps), sizeof(fps));
    uint32_t count = static_cast<uint32_t>(actions.size());
    f.write(reinterpret_cast<char const*>(&count), sizeof(count));
    for (auto const& a : actions) {
        int32_t frame = static_cast<int32_t>(std::min<uint64_t>(a.frame, INT32_MAX));
        double fw = a.window;
        uint8_t flags = a.window > 0.0 ? 1 : 0;
        if (a.player2) flags |= 2;
        int32_t ifCount = static_cast<int32_t>(std::max<uint32_t>(1, a.inputsPerFrame));
        f.write(reinterpret_cast<char const*>(&frame), sizeof(frame));
        f.write(reinterpret_cast<char const*>(&fw), sizeof(fw));
        f.write(reinterpret_cast<char const*>(&flags), sizeof(flags));
        f.write(reinterpret_cast<char const*>(&ifCount), sizeof(ifCount));
    }
}

} // namespace fwc::MacroParser
```

## `src/MacroParser.hpp`

```cpp
#pragma once
#include "Types.hpp"
#include <filesystem>
#include <vector>

namespace fwc::MacroParser {
struct Result {
    std::vector<ActionData> actions;
    double fps = 240.0;
};

geode::Result<Result> parse(std::filesystem::path const& path);
void exportFWC2(std::filesystem::path const& path, std::vector<ActionData> const& actions, double fps);
}
```

## `src/Precision.cpp`

```cpp
#include "Precision.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace fwc {

static double attemptTime(std::vector<ActionData> const& actions, double fps, double L, double& successOut) {
    if (actions.empty()) { successOut = 1.0; return 0.0; }
    constexpr double MAGIC = 0.5 * 0.7071067811865475;
    double reach = 1.0;
    double expectedFailures = 0.0;
    double endTime = actions.back().frame / fps;

    for (auto const& a : actions) {
        if (a.window <= 0.0) continue;
        double t = a.frame / fps;
        double z = (a.window / fps) * MAGIC * L;
        z = std::max(0.0, z);
        double q = std::erfc(z);
        double p = 1.0 - q;
        p = std::clamp(p, 1e-15, 1.0);
        expectedFailures += t * reach * q;
        reach *= p;
        if (reach < 1e-300) { reach = 0.0; break; }
    }

    successOut = reach;
    if (reach <= 0.0) return std::numeric_limits<double>::infinity();
    return (endTime * reach + expectedFailures) / reach;
}

PrecisionResult calculatePrecision(std::vector<ActionData> const& actions, double fps, double targetSeconds) {
    PrecisionResult r;
    if (actions.empty() || fps <= 0.0) return r;

    double sumSqMs = 0.0;
    size_t known = 0;
    std::vector<ActionData> usable;
    usable.reserve(actions.size());
    for (auto const& a : actions) {
        if (a.window <= 0.0) continue;
        double ms = a.window * 1000.0 / fps;
        sumSqMs += ms * ms;
        known++;
        usable.push_back(a);
    }
    if (!known) return r;
    r.rmsWindowMs = std::sqrt(sumSqMs / static_cast<double>(known));

    // Solve E[T_completion] ~= E[T_attempt] / P(success) = targetSeconds.
    double lo = 1e-6;
    double hi = 1.0;
    for (int i = 0; i < 80 && attemptTime(usable, fps, hi, r.successProbability) > targetSeconds; ++i) {
        hi *= 2.0;
        if (hi > 1e8) break;
    }
    for (int i = 0; i < 100; ++i) {
        double mid = (lo + hi) * 0.5;
        double p = 0.0;
        double tc = attemptTime(usable, fps, mid, p);
        if (tc > targetSeconds) lo = mid;
        else hi = mid;
    }
    r.sigmaPerSecond = (lo + hi) * 0.5;
    double p = 0.0;
    double ta = attemptTime(usable, fps, r.sigmaPerSecond, p);
    r.successProbability = p;
    r.expectedAttempts = p > 0.0 ? 1.0 / p : std::numeric_limits<double>::infinity();
    (void)ta;
    return r;
}

} // namespace fwc
```

## `src/Precision.hpp`

```cpp
#pragma once
#include "Types.hpp"

namespace fwc {

struct PrecisionResult {
    double sigmaPerSecond = 0.0;
    double rmsWindowMs = 0.0;
    double expectedAttempts = 0.0;
    double successProbability = 0.0;
};

PrecisionResult calculatePrecision(std::vector<ActionData> const& actions, double fps, double targetSeconds = 86400.0);

} // namespace fwc
```

## `src/State.cpp`

```cpp
#include "State.hpp"
#include <algorithm>

using namespace fwc;

State& State::get() {
    static State s;
    return s;
}

void State::reset() {
    std::scoped_lock lock(m_mutex);
    m_actions.clear();
    m_time = 0.0;
    m_frame = 0;
}

void State::add(ActionData action) {
    std::scoped_lock lock(m_mutex);
    action.inputIndex = static_cast<uint32_t>(m_actions.size() + 1);
    m_actions.push_back(action);
    m_frame = action.frame;
    m_time = action.timeSeconds;
}

bool State::stampLatest(double frameWindow) {
    std::scoped_lock lock(m_mutex);
    if (m_actions.empty() || frameWindow <= 0.0) return false;
    for (auto it = m_actions.rbegin(); it != m_actions.rend(); ++it) {
        if (it->window <= 0.0) {
            it->window = frameWindow;
            return true;
        }
    }
    return false;
}

bool State::importActions(std::vector<ActionData> actions, double fps) {
    if (actions.empty()) return false;
    std::scoped_lock lock(m_mutex);
    m_actions = std::move(actions);
    m_fps = fps > 0.0 ? fps : 240.0;
    m_frame = m_actions.back().frame;
    m_time = m_actions.back().timeSeconds;
    return true;
}

std::vector<ActionData> State::snapshot() const {
    std::scoped_lock lock(m_mutex);
    return m_actions;
}

Stats State::stats() const {
    std::scoped_lock lock(m_mutex);
    Stats s;
    s.total = m_actions.size();
    for (auto const& a : m_actions) {
        if (a.window <= 0.0) { s.unknown++; continue; }
        if (a.window <= 1.0) s.fw1++;
        else if (a.window <= 2.0) s.fw2++;
        else if (a.window <= 3.0) s.fw3++;
        else if (a.window <= 4.0) s.fw4++;
        else if (a.window <= 6.0) s.fw5_6++;
        else if (a.window <= 8.0) s.fw7_8++;
        else s.fw9_10++;
    }
    return s;
}

double State::fps() const { std::scoped_lock lock(m_mutex); return m_fps; }
double State::durationSeconds() const { std::scoped_lock lock(m_mutex); return m_time; }
uint64_t State::liveFrame() const { std::scoped_lock lock(m_mutex); return m_frame; }
void State::setLiveFrame(uint64_t f) { std::scoped_lock lock(m_mutex); m_frame = f; }
void State::setLiveTime(double t) { std::scoped_lock lock(m_mutex); m_time = t; }
```

## `src/State.hpp`

```cpp
#pragma once

#include "Types.hpp"
#include <mutex>
#include <optional>
#include <vector>

namespace fwc {

class State {
public:
    static State& get();

    void reset();
    void add(ActionData action);
    bool stampLatest(double frameWindow);
    bool importActions(std::vector<ActionData> actions, double fps);

    std::vector<ActionData> snapshot() const;
    Stats stats() const;
    double fps() const;
    double durationSeconds() const;
    uint64_t liveFrame() const;
    void setLiveFrame(uint64_t frame);
    void setLiveTime(double timeSeconds);

private:
    mutable std::mutex m_mutex;
    std::vector<ActionData> m_actions;
    double m_fps = 240.0;
    double m_time = 0.0;
    uint64_t m_frame = 0;
};

} // namespace fwc
```

## `src/Types.hpp`

```cpp
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
```
