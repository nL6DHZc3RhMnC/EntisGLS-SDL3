#pragma once

#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl2d/sgl_image_decoder.h>
#include <sakuragl/sgl2d/sgl_image_encoder.h>

namespace SakuraGL {

class SGLSDLImageDecoder final : public SGLImageDecoderInterface {
public:
    ESL_DECLARE_CLASS_INFO(SGLSDLImageDecoder, SGLImageDecoderInterface)
    bool IsMatchableFileExtension(const wchar_t* extension) override;
    bool IsMatchableMIMEType(const wchar_t* mime) override;
    SGLError ReadImage(SGLImageObject& image, SSystem::SFileInterface& file, size_t limitFrames = 0) override;
};

class SGLSDLImageEncoder final : public SGLImageEncoderInterface {
public:
    ESL_DECLARE_CLASS_INFO(SGLSDLImageEncoder, SGLImageEncoderInterface)
    bool IsMatchableFileExtension(const wchar_t* extension, SSystem::SString& mime) override;
    bool IsMatchableMIMEType(const wchar_t* mime) override;
    // The export pipeline supplies straight ARGB32 snapshots. Other pixel
    // formats are explicitly rejected rather than reinterpreted incorrectly.
    SGLError WriteImage(SSystem::SFileInterface& file, SGLImageObject& image,
                        const wchar_t* mime, const Options* options) override;
};

// Register after SakuraGL initializes its decoder manager, before asset loads.
void RegisterSDLImageDecoder();

} // namespace SakuraGL
