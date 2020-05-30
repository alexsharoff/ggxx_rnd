@echo off

set "script_dir=%~dp0"
set "gg_path=%script_dir%\..\.build\gg.exe"
if not exist "%gg_path%" (
    >&2 echo %gg_path% does not exist
    exit /b 1
)

start /wait %gg_path% /gamemode network /remoteip remote_ip_here /remoteport remote_port_here /localport your_port_here /side 2
