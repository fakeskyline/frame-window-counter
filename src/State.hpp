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
