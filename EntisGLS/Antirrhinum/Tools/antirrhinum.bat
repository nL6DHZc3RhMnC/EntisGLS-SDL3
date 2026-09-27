@echo off
set "ARG_LIST=%1 %2 %3 %4 %5 %6 %7 %8 %9"

:LOOP_START
shift
if "%9" == "" goto :LOOP_END
set "ARG_LIST=%ARG_LIST% %9"
goto :LOOP_START

:LOOP_END

rosetta /I %ANTIRRHINUM_HOME%\Tools %ANTIRRHINUM_HOME%\Tools\antirrhinum_compiler.rs /arg %ARG_LIST%

rem pause
