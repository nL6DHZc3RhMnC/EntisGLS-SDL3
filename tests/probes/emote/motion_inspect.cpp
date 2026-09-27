#include "extensions/emote/psb/psb_header.h"
#include "PSBRawFile.h"
#include "PSBPackedInternal.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <cmath>

static void jsonString(std::ostream &out, const std::string &s) {
    out << '"';
    for (const unsigned char c : s) {
        if (c == '"' || c == '\\') out << '\\' << c;
        else if (c < 32) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << unsigned(c) << std::dec;
        else out << c;
    }
    out << '"';
}
static void dump(std::ostream &out, const PSB::PSBRawNode &node, size_t &total, unsigned depth = 0) {
    if (++total > 2000000 || depth > 128) throw std::runtime_error("PSB tree limit exceeded");
    switch(node.GetTypeCategory()) {
        case 0: out << "null"; break;
        case 1: out << (node.GetInt() ? "true" : "false"); break;
        case 2: case 3: {
            const auto value = node.GetDouble();
            if(!std::isfinite(value)) throw std::runtime_error("non-finite PSB value");
            out << std::setprecision(17) << value;
            break;
        }
        case 4: jsonString(out, node.GetString()); break;
        case 5: {
            uint32_t size = 0;
            auto *data = node.GetResource(size);
            if(!data) throw std::runtime_error("Missing resource");
            out << "{\"$resourceBytes\":" << size << ",\"$resourceOffset\":" << (data - node.GetOwner()->GetData()) << '}';
            break;
        }
        case 6: {
            const auto *address = node.GetNode();
            const PSB::detail::PsbArray_guess offsets(address + 1);
            if (offsets.nElementCount > 2000000) throw std::runtime_error("PSB array limit exceeded");
            out << '[';
            for(uint32_t i = 0; i < offsets.nElementCount; ++i) {
                if(i) out << ',';
                dump(out, PSB::PSBRawNode(node.GetFile_guess(), address + 1 + offsets.nBytes + offsets[i]), total, depth + 1);
            }
            out << ']';
            break;
        }
        case 7: {
            out << '{'; bool first = true;
            for(const auto &key : node.GetDictionaryKeys()) {
                if(!first) out << ','; first = false;
                jsonString(out, key); out << ':';
                dump(out, node.GetDictionaryValueStrict(key.c_str()), total, depth + 1);
            }
            out << '}';
            break;
        }
        default: throw std::runtime_error("Unknown PSB category");
    }
}
int main(int argc, char **argv) {
    if(argc != 4) { std::cerr << "Usage: motion_inspect input.psb seed output.json\n"; return 2; }
    try {
        std::ifstream input(argv[1], std::ios::binary | std::ios::ate);
        if(!input) throw std::runtime_error("Cannot open PSB");
        const auto length = input.tellg();
        if(length < 56 || length > 256*1024*1024) throw std::runtime_error("PSB size out of range");
        std::vector<uint8_t> bytes(static_cast<size_t>(length));
        input.seekg(0); input.read(reinterpret_cast<char *>(bytes.data()), bytes.size());
        if(!input) throw std::runtime_error("Cannot read PSB");
        studysteady::decodePsbHeader(bytes, static_cast<uint32_t>(std::stoull(argv[2])));
        auto *data = new uint8_t[bytes.size()];
        std::memcpy(data, bytes.data(), bytes.size());
        PSB::PSBFile file;
        if(!file.Adopt(data, bytes.size())) { delete[] data; throw std::runtime_error("Cannot adopt PSB"); }
        std::ofstream out(argv[3]);
        if(!out) throw std::runtime_error("Cannot open JSON output");
        size_t total = 0; dump(out, file.GetRoot(), total); out << '\n';
        if(!out) throw std::runtime_error("Cannot write JSON");
        std::cout << "Exported " << total << " nodes\n";
    } catch(const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
}
