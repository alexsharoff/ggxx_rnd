@echo off
setlocal

call "%~dp0\_get_gg_path.bat" || exit /b 1

rem Launch processes with stream 9 redirected to a lock file.
rem The lock file will remain locked until the process exits.
9>.lock2 start "" "%gg_path%" /gamemode network /remoteip 127.0.0.1 /remoteport 7500 /localport 7501 /side 1 "%~dp0\..\ggxxacpr\test\replays\matches\slayer_vs_may.ggr"
9>.lock1 start "" "%gg_path%" /gamemode network /remoteip 127.0.0.1 /remoteport 7501 /localport 7500 /side 2 "%~dp0\..\ggxxacpr\test\replays\matches\slayer_vs_may.ggr"

rem Wait for all processes to finish (wait until lock files are no longer locked)
:wait
ping -n 1 ::1 >nul 2>&1
for %%f in (".lock*") do (
  (call ) 9>"%%f" || goto :wait
) 2>nul

rem Delete the lock files
del .lock*
