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
