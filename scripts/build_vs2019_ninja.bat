@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"
call "%~dp0\_get_build_dir.bat"
echo Build dir: %build_dir%
mkdir "%build_dir%"
pushd "%build_dir%"
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe "%~dp0\.."
cmake --build .
popd
