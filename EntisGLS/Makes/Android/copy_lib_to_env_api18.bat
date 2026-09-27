@echo off
set API_LEVEL=18
if not "%COTOPHA_HOME%" == "" xcopy /Y /S .\libs\*.* "%COTOPHA_HOME%\Library\android\api%API_LEVEL%_jni\jni\jni\libs"
pause
