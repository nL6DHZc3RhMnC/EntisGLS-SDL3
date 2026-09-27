@echo off
%~d0
cd %~dp0

set "SRC_DIR=%1"
if "%SRC_DIR%" == ""  set "SRC_DIR=%COTOPHA_HOME%\Include\common\glscs"
set "SRC_DIR2=%COTOPHA_HOME%\Include\common\rosetta"

cotoco /nologo /X /Cs /Ta %~dp0\src\dump_ECS_entries.cc /arg "%SRC_DIR%" "%SRC_DIR2%"

copy glscs_sakura2_new_object.h "%COTOPHA_HOME%\Include\common\glscs"
copy glscs_sakura2_sys_call.h "%COTOPHA_HOME%\Include\common\glscs"
pause
