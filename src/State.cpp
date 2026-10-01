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
