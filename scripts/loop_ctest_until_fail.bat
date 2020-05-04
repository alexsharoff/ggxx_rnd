@echo off

setlocal enableextensions enabledelayedexpansion

set /a count = 1

:loop
echo Run #!count!
ctest --parallel 8 --output-on-failure || exit /b 1
set ec=%errorlevel%
if [!ec!] neq [0] (
    echo Exit code !ec!
    exit /b
)
set /a count += 1
goto loop
