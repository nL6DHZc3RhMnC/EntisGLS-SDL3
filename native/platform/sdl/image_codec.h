#pragma once

#include <SDL3/SDL_iostream.h>
#include <cstdint>
#include <vector>

namespace studysteady::platform {

// Packed, top-to-bottom RGBA bytes with straight (not premultiplied) alpha.
struct RgbaImage {
    int width = 0, height = 0;
    std::vector<std::uint8_t> pixels;
};

enum class ImageEncoding { png, jpeg };

// Borrow the SDL stream; never close it. Decode reads from its current
// position through EOF and accepts PNG/JPEG signatures only. Image dimensions
// and compressed input are bounded before the codec allocates a full image.
bool DecodeImage(SDL_IOStream* input, RgbaImage& image);
// PNG preserves every RGBA byte; JPEG is lossy and drops alpha. Quality is
// 0..100 and affects JPEG only. Failure/short writes are reported to the caller.
bool EncodeImage(SDL_IOStream* output, const RgbaImage& image,
                 ImageEncoding encoding, int quality = 75);

} // namespace studysteady::platform
