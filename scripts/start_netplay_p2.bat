@echo off

call "%~dp0\_get_gg_path.bat" || exit /b 1

rem Remove last argument (replay path) to enable user control
start /wait %gg_path% /gamemode network /remoteip 127.0.0.1 /remoteport 7500 /localport 7501 /side 2 "%~dp0\..\ggxxacpr\test\replays\matches\slayer_vs_may.ggr"
