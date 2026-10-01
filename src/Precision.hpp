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
