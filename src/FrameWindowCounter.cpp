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
