@echo off
cotoco /nologo /X /Cs /Ta src\dump_SGL_new_objects.cc /arg %COTOPHA_HOME%\Source\common
copy sglx_new_object_entries.h %COTOPHA_HOME%\Include\common\sakuraglx
pause
