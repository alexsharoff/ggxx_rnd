@echo off
setlocal enableextensions enabledelayedexpansion

call "%~dp0\_get_build_dir.bat"
if not exist "%build_dir%" (
    >&2 echo %build_dir% does not exist
    exit /b 1
)

set /a count = 1
pushd %build_dir%
:loop
echo Run #!count!
ctest --parallel 6 --output-on-failure || exit /b 1
set ec=%errorlevel%
if [!ec!] neq [0] (
    echo Exit code !ec!
    popd
    exit /b
)
set /a count += 1
goto loop
