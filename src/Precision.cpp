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
