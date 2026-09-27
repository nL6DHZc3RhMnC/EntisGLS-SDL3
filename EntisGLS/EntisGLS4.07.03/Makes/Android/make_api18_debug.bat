@echo off

call call_build_jni debug
if errorlevel 1 goto :LabelExit

copy_lib_to_env_api18_debug.bat

:LabelExit
pause
