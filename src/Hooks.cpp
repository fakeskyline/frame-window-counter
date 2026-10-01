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
