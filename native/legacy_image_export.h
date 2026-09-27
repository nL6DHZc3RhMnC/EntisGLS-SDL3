#pragma once
#include <sakuragl/sakuragl.h>
class EMemoryFile;
ESLError LegacyEncodeImage(SakuraGL::SGLImageObject &,EMemoryFile &,
                          const wchar_t *mime,int quality=-1);
ESLError LegacyCreateThumbnail(SakuraGL::SGLImageObject &,SakuraGL::SGLImage &,
                              int width,int height);
