@echo off

call "%~dp0\_get_gg_path.bat" || exit /b 1

set "replay_path=%1"
if [%replay_path%] == [] (
    echo Usage: Drag and drop a .ggr file onto this script
    pause
    exit /b 1
)

start /wait %gg_path% /gamemode vs2p %1
