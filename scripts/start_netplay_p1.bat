@echo off

call "%~dp0\_get_gg_path.bat" || exit /b 1

rem Remove last argument (replay path) to enable user control
start /wait %gg_path% /gamemode network /remoteip 127.0.0.1 /remoteport 7501 /localport 7500 /side 1 "%~dp0\..\ggxxacpr\test\replays\matches\slayer_vs_may.ggr"
