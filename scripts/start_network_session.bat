@echo off
setlocal

set "script_dir=%~dp0"
set "gg_path=%script_dir%\..\.build\gg.exe"
if not exist "%gg_path%" (
    >&2 echo %gg_path% does not exist
)

rem Launch processes with stream 9 redirected to a lock file.
rem The lock file will remain locked until the process exits.
9>.lock2 start "" "%gg_path%" /gamemode network /remoteip 127.0.0.1 /remoteport 7500 /localport 7501 /side 1
9>.lock1 start "" "%gg_path%" /gamemode network /remoteip 127.0.0.1 /remoteport 7501 /localport 7500 /side 2

rem Wait for all processes to finish (wait until lock files are no longer locked)
:wait
ping -n 1 ::1 >nul 2>&1
for %%f in (".lock*") do (
  (call ) 9>"%%f" || goto :wait
) 2>nul

::delete the lock files
del .lock*
