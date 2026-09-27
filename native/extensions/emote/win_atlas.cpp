#include "extensions/emote/win_atlas.h"
#include "extensions/emote/psb/psb_header.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>

namespace studysteady::motion {
namespace {
int dimension(const PSB::PSBRawNode &node, const char *key) {
    const int value = node.GetDictionaryValueStrict(key).GetInt();
    if(value <= 0 || value > 16384) throw std::runtime_error(std::string("Invalid atlas dimension: ") + key);
    return value;
}
}

PSB::PSBFile loadPsb(const std::string &path, uint32_t seed) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if(!input) throw std::runtime_error("Cannot open PSB: " + path);
    const auto size = input.tellg();
    if(size < 56 || size > 256 * 1024 * 1024) throw std::runtime_error("PSB size out of range");
    std::vector<uint8_t> bytes(static_cast<size_t>(size));
    input.seekg(0); input.read(reinterpret_cast<char *>(bytes.data()), bytes.size());
    if(!input) throw std::runtime_error("Cannot read PSB");
    decodePsbHeader(bytes, seed);
    auto data = std::make_unique<uint8_t[]>(bytes.size());
    std::memcpy(data.get(), bytes.data(), bytes.size());
    PSB::PSBFile result;
    // Adopt owns data after success. The parser's current failure return leaves
    // it with the caller; unique_ptr handles that path.
    if(!result.Adopt(data.get(), bytes.size())) throw std::runtime_error("PSB adoption failed");
    data.release();
    return result;
}

WinAtlas::WinAtlas(const PSB::PSBRawNode &root, const std::string &group) {
    if(std::strcmp(root.GetDictionaryValueStrict("spec").GetString(), "win") != 0)
        throw std::runtime_error("WinAtlas requires a spec=win PSB");
    const auto groupNode = root.GetDictionaryValueStrict("source").GetDictionaryValueStrict(group.c_str());
    texture_ = groupNode.GetDictionaryValueStrict("texture");
    width_ = dimension(texture_, "width");
    height_ = dimension(texture_, "height");
    truncatedWidth_ = dimension(texture_, "truncated_width");
    truncatedHeight_ = dimension(texture_, "truncated_height");
    if(truncatedWidth_ > width_ || truncatedHeight_ > height_) throw std::runtime_error("Invalid truncated atlas dimensions");
    format_ = texture_.GetDictionaryValueStrict("type").GetString();
    if(format_ != "RGBA8" && format_ != "A8L8") throw std::runtime_error("Unsupported atlas format: " + format_);
    uint32_t sourceSize = 0;
    const auto *source = texture_.GetDictionaryValueStrict("pixel").GetResource(sourceSize);
    const auto *owner = texture_.GetOwner();
    const size_t channels = format_ == "RGBA8" ? 4 : 2;
    const size_t expectedSize = static_cast<size_t>(width_) * height_ * channels;
    const auto address = reinterpret_cast<uintptr_t>(source);
    const auto start = reinterpret_cast<uintptr_t>(owner->GetData());
    if(!source || sourceSize != expectedSize || address < start || address - start > size_t(owner->GetSize()) || sourceSize > size_t(owner->GetSize()) - (address - start))
        throw std::runtime_error("Atlas pixel range or size mismatch");
    if(format_ == "RGBA8") {
        rgba_ = source;
    } else {
        // Same alpha/luminance interpretation as the imported
        // PlayerResource.cpp::loadWinAtlasTexture_guess. Input is [A,L],
        // output is [L,L,L,A]. A malformed odd-size payload is rejected above.
        converted_.resize(rgbaBytes());
        for(size_t i = 0, j = 0; i < sourceSize; i += 2, j += 4) {
            converted_[j] = converted_[j + 1] = converted_[j + 2] = source[i + 1];
            converted_[j + 3] = source[i];
        }
        rgba_ = converted_.data();
    }
    const auto iconRoot = groupNode.GetDictionaryValueStrict("icon");
    for(const auto &name : iconRoot.GetDictionaryKeys()) {
        const auto node = iconRoot.GetDictionaryValueStrict(name.c_str());
        AtlasIcon icon;
        icon.name = name;
        icon.left = node.GetDictionaryValueStrict("left").GetInt();
        icon.top = node.GetDictionaryValueStrict("top").GetInt();
        icon.width = dimension(node, "width");
        icon.height = dimension(node, "height");
        icon.originX = node.GetDictionaryValueStrict("originX").GetInt();
        icon.originY = node.GetDictionaryValueStrict("originY").GetInt();
        if(icon.left < 0 || icon.top < 0 || icon.width > width_ - icon.left || icon.height > height_ - icon.top)
            throw std::runtime_error("Atlas icon out of range: " + name);
        icons_.push_back(std::move(icon));
    }
}

const AtlasIcon &WinAtlas::icon(const std::string &name) const {
    const auto found = std::find_if(icons_.begin(), icons_.end(), [&](const auto &item) { return item.name == name; });
    if(found == icons_.end()) throw std::runtime_error("Missing atlas icon: " + name);
    return *found;
}

std::vector<uint8_t> WinAtlas::copyBgra() const {
    std::vector<uint8_t> pixels(rgbaBytes());
    for(size_t i = 0; i < pixels.size(); i += 4) {
        pixels[i] = rgba_[i + 2]; pixels[i + 1] = rgba_[i + 1];
        pixels[i + 2] = rgba_[i]; pixels[i + 3] = rgba_[i + 3];
    }
    return pixels;
}

std::vector<uint8_t> WinAtlas::copyIconRgba(const AtlasIcon &item) const {
    // Accept only the object's own checked descriptor, to avoid exposing an
    // unchecked subimage reader through the public POD convenience type.
    const auto &valid = icon(item.name);
    std::vector<uint8_t> result(static_cast<size_t>(valid.width) * valid.height * 4);
    for(int row = 0; row < valid.height; ++row) {
        const auto offset = (static_cast<size_t>(valid.top + row) * width_ + valid.left) * 4;
        std::memcpy(result.data() + static_cast<size_t>(row) * valid.width * 4, rgba_ + offset, static_cast<size_t>(valid.width) * 4);
    }
    return result;
}
}
