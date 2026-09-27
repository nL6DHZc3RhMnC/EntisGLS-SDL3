#include "platform/sdl/image_codec.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <memory>
#include <new>

// The SDK's tinygltf translation unit also implements stb. Keep this narrower
// JPEG decoder/encoder private to this translation unit to avoid symbol and
// global-setting collisions. PNG uses SDL 3.4's native IO codec.
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_NO_STDIO
#define STBI_MAX_DIMENSIONS 32768
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif
#include <stb_image.h>
#include <stb_image_write.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

namespace studysteady::platform {
namespace {
constexpr std::size_t maxEncodedBytes = 256u * 1024u * 1024u;
constexpr std::uint64_t maxPixels = 0x4000000u;
using Surface = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;
using IO = std::unique_ptr<SDL_IOStream, decltype(&SDL_CloseIO)>;

bool Dimensions(int width, int height) {
    return width > 0 && height > 0 && width <= 32768 && height <= 32768
        && std::uint64_t(width) * height <= maxPixels;
}

bool ReadAll(SDL_IOStream* input, std::vector<std::uint8_t>& bytes) {
    if (!input) return SDL_SetError("null image input");
    std::array<std::uint8_t, 32768> block;
    for (;;) {
        const auto count = SDL_ReadIO(input, block.data(), block.size());
        if (count > maxEncodedBytes - bytes.size()) return SDL_SetError("compressed image exceeds 256 MiB limit");
        bytes.insert(bytes.end(), block.data(), block.data() + count);
        const auto status = SDL_GetIOStatus(input);
        if (status == SDL_IO_STATUS_EOF) return !bytes.empty() || SDL_SetError("empty image input");
        if (status != SDL_IO_STATUS_READY || !count) return SDL_SetError("failed to read complete image input");
    }
}

std::uint32_t BigEndian32(const std::uint8_t* p) {
    return (std::uint32_t(p[0]) << 24) | (std::uint32_t(p[1]) << 16)
        | (std::uint32_t(p[2]) << 8) | std::uint32_t(p[3]);
}

struct JpegOutput {
    SDL_IOStream* stream;
    bool failed = false;
    static void Write(void* opaque, void* bytes, int count) {
        auto& self = *static_cast<JpegOutput*>(opaque);
        if (!self.failed && (count < 0 || SDL_WriteIO(self.stream, bytes, std::size_t(count)) != std::size_t(count)))
            self.failed = true;
    }
};
}

bool DecodeImage(SDL_IOStream* input, RgbaImage& image) try {
    std::vector<std::uint8_t> encoded;
    if (!ReadAll(input, encoded)) return false;
    static constexpr std::uint8_t pngSignature[] = {137, 80, 78, 71, 13, 10, 26, 10};
    RgbaImage decoded;
    if (encoded.size() >= 24 && !std::memcmp(encoded.data(), pngSignature, sizeof(pngSignature))) {
        const auto width = BigEndian32(encoded.data() + 16), height = BigEndian32(encoded.data() + 20);
        if (std::memcmp(encoded.data() + 12, "IHDR", 4) || width > 32768 || height > 32768
            || !Dimensions(int(width), int(height))) return SDL_SetError("invalid or excessive PNG dimensions");
        IO memory(SDL_IOFromConstMem(encoded.data(), encoded.size()), &SDL_CloseIO);
        if (!memory) return false;
        Surface source(SDL_LoadPNG_IO(memory.get(), false), &SDL_DestroySurface);
        if (!source) return false;
        if (!Dimensions(source->w, source->h)) return SDL_SetError("invalid decoded PNG dimensions");
        Surface rgba(SDL_ConvertSurface(source.get(), SDL_PIXELFORMAT_RGBA32), &SDL_DestroySurface);
        if (!rgba) return false;
        decoded.width = rgba->w;
        decoded.height = rgba->h;
        decoded.pixels.resize(std::size_t(rgba->w) * rgba->h * 4);
        if (!SDL_LockSurface(rgba.get())) return false;
        for (int y = 0; y < rgba->h; ++y)
            std::memcpy(decoded.pixels.data() + std::size_t(y) * rgba->w * 4,
                        static_cast<const std::uint8_t*>(rgba->pixels) + std::ptrdiff_t(y) * rgba->pitch,
                        std::size_t(rgba->w) * 4);
        SDL_UnlockSurface(rgba.get());
    } else if (encoded.size() >= 3 && encoded[0] == 0xff && encoded[1] == 0xd8 && encoded[2] == 0xff) {
        int channels = 0;
        if (!stbi_info_from_memory(encoded.data(), int(encoded.size()), &decoded.width, &decoded.height, &channels)
            || !Dimensions(decoded.width, decoded.height)) return SDL_SetError("invalid or excessive JPEG dimensions");
        std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> pixels(
            stbi_load_from_memory(encoded.data(), int(encoded.size()), &decoded.width, &decoded.height, &channels, 4),
            &stbi_image_free);
        if (!pixels) return SDL_SetError("JPEG decode failed: %s", stbi_failure_reason());
        decoded.pixels.assign(pixels.get(), pixels.get() + std::size_t(decoded.width) * decoded.height * 4);
    } else return SDL_SetError("image stream is neither PNG nor JPEG");
    image = std::move(decoded);
    return true;
} catch (const std::bad_alloc&) {
    return SDL_SetError("out of memory decoding image");
}

bool EncodeImage(SDL_IOStream* output, const RgbaImage& image, ImageEncoding encoding, int quality) try {
    if (!output || !Dimensions(image.width, image.height)
        || image.pixels.size() != std::size_t(image.width) * image.height * 4)
        return SDL_SetError("invalid RGBA image for encoding");
    if (quality < 0 || quality > 100) return SDL_SetError("image quality must be 0..100");
    if (encoding == ImageEncoding::png) {
        Surface surface(SDL_CreateSurfaceFrom(image.width, image.height, SDL_PIXELFORMAT_RGBA32,
            const_cast<std::uint8_t*>(image.pixels.data()), image.width * 4), &SDL_DestroySurface);
        // SDL 3.4.16's PNG writer tests WriteIO for nonzero, not for a full
        // write (src/video/SDL_stb.c). Encode into SDL's seekable memory stream
        // first, then check the exact byte count on the caller's actual output.
        IO encoded(SDL_IOFromDynamicMem(), &SDL_CloseIO);
        if (!surface || !encoded || !SDL_SavePNG_IO(surface.get(), encoded.get(), false)) return false;
        const auto size = SDL_GetIOSize(encoded.get());
        const auto* bytes = SDL_GetPointerProperty(SDL_GetIOProperties(encoded.get()),
            SDL_PROP_IOSTREAM_DYNAMIC_MEMORY_POINTER, nullptr);
        if (size <= 0 || !bytes || std::uint64_t(size) > std::numeric_limits<std::size_t>::max())
            return SDL_SetError("PNG encoder returned no complete output");
        if (SDL_WriteIO(output, bytes, std::size_t(size)) != std::size_t(size))
            return SDL_SetError("PNG output failed or was incomplete");
        return true;
    }
    if (encoding != ImageEncoding::jpeg) return SDL_SetError("unsupported image encoding");
    JpegOutput target{output};
    // stb treats zero as its default quality; map requested zero to its lowest
    // actual quality instead, preserving the caller's 0..100 ordering.
    const bool success = stbi_write_jpg_to_func(&JpegOutput::Write, &target,
        image.width, image.height, 4, image.pixels.data(), std::max(quality, 1)) != 0;
    if (!success || target.failed) return SDL_SetError("JPEG output failed or was incomplete");
    return true;
} catch (const std::bad_alloc&) {
    return SDL_SetError("out of memory encoding image");
}

} // namespace studysteady::platform
