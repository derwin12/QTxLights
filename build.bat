@echo off
setlocal

set QT_DIR=D:\Qt\6.10.2\mingw_64
set CMAKE=D:\Qt\Tools\CMake_64\bin\cmake.exe
set MINGW=D:\Qt\Tools\mingw1310_64\bin
set NINJA=D:\Qt\Tools\Ninja

set PATH=%MINGW%;%QT_DIR%\bin;%NINJA%;%PATH%

if not exist build mkdir build
pushd build

"%CMAKE%" .. -G Ninja -DCMAKE_PREFIX_PATH="%QT_DIR%" -DCMAKE_BUILD_TYPE=Debug
"%CMAKE%" --build . --parallel

popd
echo.
echo Done. Run: build\QTxLights.exe
