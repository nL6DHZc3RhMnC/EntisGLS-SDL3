#include "../image_codec.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>

using namespace studysteady::platform;
using IO = std::unique_ptr<SDL_IOStream, decltype(&SDL_CloseIO)>;

namespace {
void Require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(std::string(message) + ": " + SDL_GetError());
}

RgbaImage Fixture(int width, int height) {
    RgbaImage image;
    image.width = width;
    image.height = height;
    image.pixels.resize(std::size_t(width) * height * 4);
    constexpr unsigned char alphas[] = {0, 64, 128, 255};
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
        auto* p = image.pixels.data() + (std::size_t(y) * width + x) * 4;
        p[0] = 40 + x * 120 / width;
        p[1] = 170 - y * 100 / height;
        p[2] = 210 - x * 80 / width;
        p[3] = alphas[(x + y) % 4];
    }
    return image;
}

RgbaImage RoundTrip(const RgbaImage& source, ImageEncoding encoding, int quality) {
    IO file(SDL_IOFromDynamicMem(), &SDL_CloseIO);
    Require(bool(file), "create encoded memory file");
    // Exercise a nonzero stream origin rather than assuming all images begin
    // at offset zero in an archive or SDK memory file.
    const char prefix[] = "archive prefix";
    Require(SDL_WriteIO(file.get(), prefix, sizeof(prefix)) == sizeof(prefix), "write prefix");
    Require(EncodeImage(file.get(), source, encoding, quality), "encode image");
    Require(SDL_GetIOSize(file.get()) > 30, "encoder must produce real bytes");
    Require(SDL_SeekIO(file.get(), sizeof(prefix), SDL_IO_SEEK_SET) == sizeof(prefix), "seek embedded image");
    RgbaImage decoded;
    Require(DecodeImage(file.get(), decoded), "decode embedded image");
    return decoded;
}
}

int main() {
    try {
        const auto source = Fixture(37, 19);
        const auto png = RoundTrip(source, ImageEncoding::png, 75);
        Require(png.width == source.width && png.height == source.height, "PNG dimensions");
        Require(png.pixels == source.pixels, "PNG must preserve all straight RGBA bytes including transparent RGB");
        std::cout << "SDL PNG round trip, odd pitch, straight alpha and transparent RGB: PASS\n";

        for (const int quality : {0, 75, 100}) {
            const auto jpeg = RoundTrip(source, ImageEncoding::jpeg, quality);
            Require(jpeg.width == source.width && jpeg.height == source.height, "JPEG dimensions");
            double colorError = 0;
            for (std::size_t i = 0; i < jpeg.pixels.size(); i += 4) {
                Require(jpeg.pixels[i + 3] == 255, "JPEG output must decode as opaque");
                for (int c = 0; c < 3; ++c) colorError += std::abs(int(jpeg.pixels[i + c]) - source.pixels[i + c]);
            }
            colorError /= source.width * source.height * 3;
            Require(colorError < (quality == 0 ? 45 : 8), "JPEG colors must remain within lossy tolerance");
        }
        std::cout << "private stb JPEG round trip, quality bounds and opaque alpha: PASS\n";

        SDL_IOStreamInterface sink{};
        SDL_INIT_INTERFACE(&sink);
        sink.write = [](void*, const void*, size_t bytes, SDL_IOStatus* status) -> size_t {
            *status = SDL_IO_STATUS_ERROR;
            return std::min<std::size_t>(bytes, 1);
        };
        IO shortOutput(SDL_OpenIO(&sink, nullptr), &SDL_CloseIO);
        Require(!EncodeImage(shortOutput.get(), source, ImageEncoding::png, 75), "PNG short write must fail");
        Require(!EncodeImage(shortOutput.get(), source, ImageEncoding::jpeg, 75), "JPEG short write must fail");
        Require(!EncodeImage(shortOutput.get(), source, ImageEncoding::jpeg, -1), "invalid quality must fail");
        const unsigned char invalid[] = {137, 80, 78, 71};
        IO shortInput(SDL_IOFromConstMem(invalid, sizeof(invalid)), &SDL_CloseIO);
        RgbaImage untouched = source;
        Require(!DecodeImage(shortInput.get(), untouched) && untouched.pixels == source.pixels, "decode failure must not publish an image");
        std::cout << "invalid input, failed output and unchanged failed result: PASS\n";

        SDL_Quit();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
