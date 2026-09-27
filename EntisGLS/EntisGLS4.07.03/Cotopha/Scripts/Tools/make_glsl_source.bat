@echo off
if not exist glsl.bin  md glsl.bin
del /Q glsl.bin\*.*
scxcopy /nologo /SD /Es shift_jis /Ed utf-8 "%COTOPHA_HOME%\Source\glsl" glsl.bin
noa32c /nologo /p /naked glsl.bin\*.* glsl.bin
cotoco /nologo /X /Cs src\make_glsl_source.cc /arg "%COTOPHA_HOME%\Include\opengl\sakuragl\glsl_src_bin.h" "%COTOPHA_HOME%\Include\opengl\sakuragl\glsl_src_bin.hpp" glsl.bin\*.bin
pause
