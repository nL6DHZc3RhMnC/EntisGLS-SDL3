#include "platform/sdl/sdk_image_codec.h"
#include "platform/sdl/image_codec.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>
#include <new>

using namespace SakuraGL;

ESL_IMPLEMENT_CLASS_INFO(SakuraGL::SGLSDLImageDecoder, SGLImageDecoderInterface)
ESL_IMPLEMENT_CLASS_INFO(SakuraGL::SGLSDLImageEncoder, SGLImageEncoderInterface)

namespace {
bool Equal(const wchar_t* left, const wchar_t* right) {
    return left && SSystem::SString::CompareNoCase(left, right) == 0;
}

// Borrow SFileInterface rather than guessing a filesystem path: NOA members,
// memory files, and imported resources all keep their existing SDK IO route.
class FileIO {
public:
    explicit FileIO(SSystem::SFileInterface& file) : file_(file) {
        SDL_IOStreamInterface callbacks{};
        SDL_INIT_INTERFACE(&callbacks);
        callbacks.size = [](void* opaque) -> Sint64 { return static_cast<FileIO*>(opaque)->file_.GetLength(); };
        callbacks.seek = [](void* opaque, Sint64 offset, SDL_IOWhence whence) -> Sint64 {
            auto& self = *static_cast<FileIO*>(opaque);
            if (!self.file_.IsSeekable()) { SDL_SetError("SDK image stream is not seekable"); return -1; }
            SSystem::SFileInterface::SeekOrigin origin;
            switch (whence) {
            case SDL_IO_SEEK_SET: origin = SSystem::SFileInterface::FromBegin; break;
            case SDL_IO_SEEK_CUR: origin = SSystem::SFileInterface::FromCurrent; break;
            case SDL_IO_SEEK_END: origin = SSystem::SFileInterface::FromEnd; break;
            default: SDL_SetError("invalid image stream seek origin"); return -1;
            }
            const auto position = self.file_.Seek(offset, origin);
            if (position < 0) self.failed = true;
            return position;
        };
        callbacks.read = [](void* opaque, void* bytes, size_t count, SDL_IOStatus* status) -> size_t {
            auto& self = *static_cast<FileIO*>(opaque);
            const auto read = self.file_.Read(bytes, count);
            if (read < count) {
                const auto position = self.file_.GetPosition(), length = self.file_.GetLength();
                // SFileInterface exposes no separate error/EOF status. Known
                // length lets us reject a short read before the expected end;
                // unknown-length streams use a zero read as their EOF signal.
                if (read == 0 && length >= 0 && position >= 0 && position < length) {
                    self.failed = true;
                    *status = SDL_IO_STATUS_ERROR;
                } else if (read == 0 || (length >= 0 && position >= length)) *status = SDL_IO_STATUS_EOF;
            }
            return read;
        };
        callbacks.write = [](void* opaque, const void* bytes, size_t count, SDL_IOStatus* status) -> size_t {
            auto& self = *static_cast<FileIO*>(opaque);
            const auto written = self.file_.Write(bytes, count);
            if (written != count) { self.failed = true; *status = SDL_IO_STATUS_ERROR; }
            return written;
        };
        stream.reset(SDL_OpenIO(&callbacks, this));
    }
    std::unique_ptr<SDL_IOStream, decltype(&SDL_CloseIO)> stream{nullptr, &SDL_CloseIO};
    bool failed = false;
private:
    SSystem::SFileInterface& file_;
};
}

bool SGLSDLImageDecoder::IsMatchableFileExtension(const wchar_t* extension) {
    return Equal(extension, L"png") || Equal(extension, L"jpg") || Equal(extension, L"jpeg");
}
bool SGLSDLImageDecoder::IsMatchableMIMEType(const wchar_t* mime) {
    return Equal(mime, L"image/png") || Equal(mime, L"image/jpeg");
}

SGLError SGLSDLImageDecoder::ReadImage(SGLImageObject& image, SSystem::SFileInterface& file, size_t) try {
    FileIO input(file);
    studysteady::platform::RgbaImage decoded;
    if (!input.stream || !studysteady::platform::DecodeImage(input.stream.get(), decoded) || input.failed)
        return sglErrFailed;
    const auto format = formatImageARGB | formatImageFlagNoProductOfAlpha;
    if (image.CreateImage(decoded.width, decoded.height, format, 32, SGLImageObject::bufferOnMemory))
        return sglErrFailed;
    SGLImageInfo info;
    auto* destination = image.LockBuffer(info, SGLImageObject::lockWrite);
    if (!destination) return sglErrFailed;
    if (info.depth != 32 || info.pitchPixel != 4) {
        image.UnlockBuffer(SGLImageObject::lockWrite);
        return sglErrFailed;
    }
    for (int y = 0; y < decoded.height; ++y) {
        auto* row = destination + std::int64_t(y) * info.pitchLine;
        const auto* source = decoded.pixels.data() + std::size_t(y) * decoded.width * 4;
        for (int x = 0; x < decoded.width; ++x) {
            // SDK ARGB32 has B,G,R,A bytes. RGBA is the codec's explicit wire
            // layout; do not alias it as an SDK packed color integer.
            row[x * 4 + 0] = source[x * 4 + 2];
            row[x * 4 + 1] = source[x * 4 + 1];
            row[x * 4 + 2] = source[x * 4 + 0];
            row[x * 4 + 3] = source[x * 4 + 3];
        }
    }
    return image.UnlockBuffer(SGLImageObject::lockWrite);
} catch (const std::bad_alloc&) {
    return sglErrFailed;
}

bool SGLSDLImageEncoder::IsMatchableFileExtension(const wchar_t* extension, SSystem::SString& mime) {
    if (Equal(extension, L"png")) { mime = L"image/png"; return true; }
    if (Equal(extension, L"jpg") || Equal(extension, L"jpeg")) { mime = L"image/jpeg"; return true; }
    return false;
}
bool SGLSDLImageEncoder::IsMatchableMIMEType(const wchar_t* mime) {
    return Equal(mime, L"image/png") || Equal(mime, L"image/jpeg");
}

SGLError SGLSDLImageEncoder::WriteImage(SSystem::SFileInterface& file, SGLImageObject& image,
                                      const wchar_t* mime, const Options* options) try {
    if (!IsMatchableMIMEType(mime)) return sglErrNotSupported;
    SGLImageInfo info;
    const auto required = formatImageARGB | formatImageFlagNoProductOfAlpha;
    if (image.GetImageInfo(info) || !info.width || !info.height || std::uint64_t(info.width) * info.height > 0x4000000u
        || info.format != required || info.depth != 32 || info.pitchPixel != 4) return sglErrNotSupported;
    studysteady::platform::RgbaImage rgba;
    rgba.width = int(info.width);
    rgba.height = int(info.height);
    rgba.pixels.resize(std::size_t(info.width) * info.height * 4);
    const auto* source = image.LockBuffer(info, SGLImageObject::lockRead);
    if (!source) return sglErrFailed;
    for (unsigned y = 0; y < info.height; ++y) {
        const auto* row = source + std::int64_t(y) * info.pitchLine;
        auto* destination = rgba.pixels.data() + std::size_t(y) * info.width * 4;
        for (unsigned x = 0; x < info.width; ++x) {
            destination[x * 4 + 0] = row[x * 4 + 2];
            destination[x * 4 + 1] = row[x * 4 + 1];
            destination[x * 4 + 2] = row[x * 4 + 0];
            destination[x * 4 + 3] = row[x * 4 + 3];
        }
    }
    if (image.UnlockBuffer(SGLImageObject::lockRead)) return sglErrFailed;
    const auto encoding = Equal(mime, L"image/png") ? studysteady::platform::ImageEncoding::png
                                                  : studysteady::platform::ImageEncoding::jpeg;
    const int quality = options && (options->nFlags & optionQuality)
        ? int(std::min<std::uint32_t>(256, options->nQuality) * 100 + 128) / 256 : 75;
    FileIO output(file);
    return output.stream && studysteady::platform::EncodeImage(output.stream.get(), rgba, encoding, quality) && !output.failed
        ? sglErrSuccess : sglErrFailed;
} catch (const std::bad_alloc&) {
    return sglErrFailed;
}

void SakuraGL::RegisterSDLImageDecoder() {
    // Preserve SDK ERI/BMP/TGA/PSD defaults even if the caller registers early.
    SGLImageDecoderManager::Initialzie();
    SGLImageDecoderManager::RegisterDecoder(new SGLSDLImageDecoder);
}
