#pragma once
#include "extensions/emote/win_atlas.h"
#include <GLES2/gl2.h>

namespace studysteady::motion {
// Call only with a current GLES context. The returned texture belongs to the
// caller and must be deleted with glDeleteTextures on that context. The upload
// preserves GL_TEXTURE_BINDING_2D and GL_UNPACK_ALIGNMENT.
GLuint uploadWinAtlasGles(const WinAtlas &atlas);
}
