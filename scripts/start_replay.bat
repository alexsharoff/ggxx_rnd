@echo off

set "script_dir=%~dp0"
set "gg_path=%script_dir%\..\.build\gg.exe"
if not exist "%gg_path%" (
    >&2 echo %gg_path% does not exist
    exit /b 1
)

set "replay_path=%1"
if [%replay_path%] == [] (
    echo Usage: Drag and drop a .ggr file onto this script
    pause
    exit /b 1
)

start /wait %gg_path% /gamemode vs2p %1
