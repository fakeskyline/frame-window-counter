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
