@echo off
setlocal enableextensions enabledelayedexpansion

set "script_dir=%~dp0"
set "build_dir=%script_dir%\..\.build"
if not exist "%build_dir%" (
    >&2 echo %build_dir% does not exist
)

set /a count = 1
pushd %build_dir%
:loop
echo Run #!count!
ctest --parallel 8 --output-on-failure || exit /b 1
set ec=%errorlevel%
if [!ec!] neq [0] (
    echo Exit code !ec!
    popd
    exit /b
)
set /a count += 1
goto loop
