echo off

if "%ANDROID_NDK_HOME%" == "" goto :Label_ErrorNoNdkHome
goto :Label_Start


:Label_Start
rem set OPT_DEBUG="NDK_DEBUG=0"
set OPT_DEBUG=
if "%1" == "debug" set OPT_DEBUG="NDK_DEBUG=1"

set "SAVE_PATH=%path%"

rem Cygwin での実行
rem set path=%CYGWIN_HOME%\bin
rem bash %ANDROID_NDK_CYGWIN_PATH%/ndk-build %OPT_DEBUG% %2

rem Windows コマンドでの実行
set "path=%ANDROID_NDK_HOME%;%path%"
rem call ndk-build %OPT_DEBUG% NDK_LOG=1 V=1 %2
call ndk-build %OPT_DEBUG% %2
if errorlevel 1 goto :LabelExit

set "path=%SAVE_PATH%"

echo ファイルをコピーします
if exist libs (
	del /Q /S libs\*.*
) else (
	md libs
)
for /D %%I in (.\obj\local\*.*) do (
	if exist %%I\libgls4.a (
		if exist .\libs\%%~nI (del /Q .\libs\%%~nI\*.*) else (md .\libs\%%~nI)
		xcopy /Y /S %%I\libgls4.a .\libs\%%~nI
	)
)

goto :LabelExit



:Label_ErrorNoNdkHome
echo 環境変数 ANDROID_NDK_HOME に Android NDK のルートディレクトリを設定してください
set /P "ANDROID_NDK_HOME=Android NDK ディレクトリ："

if "%ANDROID_NDK_HOME%" == "" goto :LabelExit
goto :Label_Start


:LabelExit
rem if "%1" == "" pause
