#include "extensions/emote/win_atlas.h"
#include <fstream>
#include <iostream>
#include <zlib.h>

int main(int argc, char **argv) {
    if(argc < 4 || argc > 6) {
        std::cerr << "Usage: motion_texture_probe file.psb seed group [output.pam [icon]]\n";
        return 2;
    }
    try {
        studysteady::motion::WinAtlas atlas(
            studysteady::motion::loadPsb(argv[1], static_cast<uint32_t>(std::stoull(argv[2]))).GetRoot(), argv[3]);
        // The temporary PSBFile is already destroyed here. This also exercises
        // retained raw-owner lifetime, which the actual renderer depends on.
        const auto checksum = crc32(0, atlas.rgbaPixels(), static_cast<uInt>(atlas.rgbaBytes()));
        std::cout << "atlas=" << atlas.width() << 'x' << atlas.height()
                  << " format=" << atlas.format() << " icons=" << atlas.icons().size()
                  << " rgba_bytes=" << atlas.rgbaBytes() << " rgba_crc32="
                  << std::hex << checksum << std::dec << '\n';
        size_t opaque = 0, nonzero = 0;
        for(size_t i = 3; i < atlas.rgbaBytes(); i += 4) {
            nonzero += atlas.rgbaPixels()[i] != 0;
            opaque += atlas.rgbaPixels()[i] == 255;
        }
        if(!nonzero) throw std::runtime_error("Atlas is completely transparent");
        std::cout << "visible_pixels=" << nonzero << " opaque_pixels=" << opaque << '\n';
        if(argc >= 5) {
            int width = atlas.width(), height = atlas.height();
            const uint8_t *pixels = atlas.rgbaPixels();
            std::vector<uint8_t> cropped;
            if(argc == 6) {
                const auto &icon = atlas.icon(argv[5]);
                width = icon.width; height = icon.height;
                cropped = atlas.copyIconRgba(icon); pixels = cropped.data();
                std::cout << "icon=" << icon.name << " origin=" << icon.originX << ',' << icon.originY << '\n';
            }
            std::ofstream output(argv[4], std::ios::binary);
            output << "P7\nWIDTH " << width << "\nHEIGHT " << height << "\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
            output.write(reinterpret_cast<const char *>(pixels), static_cast<size_t>(width) * height * 4);
            if(!output) throw std::runtime_error("Cannot write PAM");
            std::cout << "wrote=" << argv[4] << '\n';
        }
        return 0;
    } catch(const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
}
