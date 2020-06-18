@echo off

call "%~dp0\_get_build_dir.bat"
echo Build dir: %build_dir%
mkdir "%build_dir%"
pushd "%build_dir%"
cmake -G "Visual Studio 16 2019" -A Win32 "%~dp0\.."
cmake --build . --config Release --parallel
popd
