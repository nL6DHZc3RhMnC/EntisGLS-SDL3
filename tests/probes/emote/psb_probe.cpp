#include "extensions/emote/psb/psb_header.h"
#include "PSBRawFile.h"
#include "PSBPackedInternal.h"
#include "EmoteAngleController.h"
#include "EmoteVarController.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <cmath>
#include <map>

static void inspectNode(const PSB::PSBRawNode &node, std::map<unsigned, size_t> &counts,
                        size_t &total, unsigned depth = 0) {
    if (++total > 2000000 || depth > 128) throw std::runtime_error("PSB tree limit exceeded");
    const auto *owner = node.GetOwner();
    const auto *address = node.GetNode();
    if (address < owner->GetData() || address >= owner->GetData() + owner->GetSize())
        throw std::runtime_error("PSB node out of bounds");
    const auto type = node.GetType();
    ++counts[type];
    if (type == 0x21) {
        for (const auto &key : node.GetDictionaryKeys())
            inspectNode(node.GetDictionaryValueStrict(key.c_str()), counts, total, depth + 1);
    } else if (type == 0x20) {
        const PSB::detail::PsbArray_guess offsets(address + 1);
        if (offsets.nElementCount > 2000000) throw std::runtime_error("PSB array too large");
        for (uint32_t i = 0; i < offsets.nElementCount; ++i)
            inspectNode(PSB::PSBRawNode(node.GetFile_guess(), address + 1 + offsets.nBytes + offsets[i]),
                        counts, total, depth + 1);
    } else if (type >= 0x22 && type <= 0x25) {
        uint32_t size = 0;
        if (node.GetTypeCategory() != 5 || !node.GetResource(size))
            throw std::runtime_error("PSB v4 resource lookup failed");
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: psb_probe file.psb header_seed\n";
        return 2;
    }
    try {
        std::ifstream stream(argv[1], std::ios::binary | std::ios::ate);
        if (!stream) throw std::runtime_error("Cannot open input");
        const auto length = stream.tellg();
        if (length < 56 || length > 256 * 1024 * 1024)
            throw std::runtime_error("PSB size out of range");
        stream.seekg(0);
        std::vector<uint8_t> bytes(static_cast<size_t>(length));
        stream.read(reinterpret_cast<char *>(bytes.data()), bytes.size());
        if (!stream) throw std::runtime_error("PSB read failed");
        studysteady::decodePsbHeader(bytes, static_cast<uint32_t>(std::stoull(argv[2])));
        PSB::PSBFile file;
        auto *data = new uint8_t[bytes.size()];
        std::memcpy(data, bytes.data(), bytes.size());
        if (!file.Adopt(data, bytes.size())) {
            delete[] data;
            throw std::runtime_error("PSB adoption failed");
        }
        auto root = file.GetRoot();
        std::cout << "Header checksum: PASS\nRoot members:";
        for (const auto &key : root.GetDictionaryKeys()) std::cout << " " << key;
        std::cout << "\n";
        for (const auto &key : {"spec", "id", "version"}) {
            PSB::PSBRawNode value;
            if (root.GetDictionaryValue(key, value)) {
                std::cout << key << ": ";
                if (value.GetTypeCategory() == 4) std::cout << value.GetString();
                else std::cout << value.GetDouble();
                std::cout << "\n";
            }
        }
        PSB::PSBRawNode objects;
        if (root.GetDictionaryValue("object", objects)) {
            std::cout << "Object members:";
            for (const auto &key : objects.GetDictionaryKeys()) std::cout << " " << key;
            std::cout << "\n";
        }
        std::map<unsigned, size_t> counts;
        size_t total = 0;
        inspectNode(root, counts, total);
        std::cout << "PSB v4 extra-resource traversal: PASS\n";
        std::cout << "Traversed nodes: " << total << "\nTypes:";
        for (auto [tag, count] : counts) std::cout << " " << std::hex << tag << std::dec << ":" << count;
        std::cout << "\n";
        // Exercise the actual imported interpolation code on both host/ARM64.
        motion::EmoteVarController ctl(2);
        const float target[2] = {10.f, 20.f};
        motion::EmoteVarController_setTarget_guess(&ctl, target, 1.f, 1.f, false);
        float out[2];
        motion::EmoteVarController_step(&ctl, out, .5f);
        if (std::fabs(out[0] - 5.f) > 1e-6f || std::fabs(out[1] - 10.f) > 1e-6f)
            throw std::runtime_error("Motion controller interpolation failed");
        std::cout << "Imported MotionPlayer interpolation: PASS\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
