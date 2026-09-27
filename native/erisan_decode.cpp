#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <vector>

class FileInput final : public SSystem::SInputStream {
    std::ifstream file;
public:
    explicit FileInput(const char *path) : file(path, std::ios::binary) {
        if (!file) throw std::runtime_error("Cannot open input");
    }
    size_t Read(void *buffer, size_t count) override {
        file.read(static_cast<char *>(buffer), count);
        return static_cast<size_t>(file.gcount());
    }
};

int main(int argc, char **argv) {
    if (argc < 3 || argc > 4) {
        std::fprintf(stderr, "Usage: erisan_decode input output [expected_bytes]\n");
        return 2;
    }
    try {
        const size_t limit = argc == 4 ? std::stoull(argv[3]) : 256 * 1024 * 1024;
        if (limit > 256 * 1024 * 1024) throw std::runtime_error("Output limit exceeded");
        FileInput input(argv[1]);
        ERISA::SGLDecodeBitStream bits(0x4000);
        bits.AttachInputStream(&input);
        ERISA::SGLERISANDecodeContext decoder(&bits);
        decoder.PrepareToDecodeERISANCode();
        std::ofstream output(argv[2], std::ios::binary);
        if (!output) throw std::runtime_error("Cannot open output");
        std::vector<char> block(65536);
        size_t total = 0;
        while (total < limit) {
            const size_t got = decoder.Read(block.data(), std::min(block.size(), limit - total));
            if (got == 0) break;
            output.write(block.data(), got);
            if (!output) throw std::runtime_error("Write failed");
            total += got;
        }
        if (argc == 4 && total != limit) throw std::runtime_error("Decoded size mismatch");
        if (argc != 4 && total == limit) throw std::runtime_error("Missing EOF / output limit reached");
        std::printf("Decoded %zu bytes\n", total);
    } catch (const std::exception &error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
