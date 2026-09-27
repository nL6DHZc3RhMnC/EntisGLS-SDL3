@echo off

call call_build_jni release
if errorlevel 1 goto :LabelExit

copy_lib_to_env_api18.bat

:LabelExit
pause
