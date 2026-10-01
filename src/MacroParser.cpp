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
