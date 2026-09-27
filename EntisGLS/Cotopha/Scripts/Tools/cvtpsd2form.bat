@echo off
for %%I in (src\*.*) do (

if not exist dst\%%~nI md dst\%%~nI
del /Q dst\%%~nI\*.*

rosetta cvtpsd2form.rs /arg /dst_dir dst\%%~nI /form dst\%%~nI\%%~nI.xmlfrm /comp dst\%%~nI\%%~nI.xmlprs %%I

)
pause
