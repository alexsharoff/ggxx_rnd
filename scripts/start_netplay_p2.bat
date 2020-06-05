@echo off

set "script_dir=%~dp0"
set "gg_path=%script_dir%\..\.build\gg.exe"
if not exist "%gg_path%" (
    >&2 echo %gg_path% does not exist
    exit /b 1
)

start /wait %gg_path% /gamemode network /remoteip 127.0.0.1 /remoteport 7500 /localport 7501 /side 2
