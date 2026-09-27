@echo off
%~d0
cd %~dp0
rem cotoco /nologo /X /Cs /Ta %~dp0\src\MoviePlayer.cc /arg %1
rosetta %~dp0\src\MoviePlayer.rs /arg %1
pause
