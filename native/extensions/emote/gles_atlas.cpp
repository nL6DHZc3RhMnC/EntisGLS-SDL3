#include "extensions/emote/gles_atlas.h"
#include <stdexcept>
#include <string>

namespace studysteady::motion {
GLuint uploadWinAtlasGles(const WinAtlas &atlas) {
    GLint maxSize = 0, previousTexture = 0, previousAlignment = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
    if(maxSize < atlas.width() || maxSize < atlas.height()) throw std::runtime_error("Atlas exceeds GLES texture size limit");
    if(glGetError() != GL_NO_ERROR) throw std::runtime_error("GLES context has a pending error before atlas upload");
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas.width(), atlas.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas.rgbaPixels());
    const GLenum error = glGetError();
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));
    glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);
    if(error != GL_NO_ERROR) {
        glDeleteTextures(1, &texture);
        throw std::runtime_error("GLES atlas upload failed: " + std::to_string(error));
    }
    return texture;
}
}
