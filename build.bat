@echo off
setlocal EnableDelayedExpansion

set "QT_DIR=D:/Qt/6.10.2/mingw_64"
set "CMAKE=D:/Qt/Tools/CMake_64/bin/cmake.exe"
set "MINGW=D:/Qt/Tools/mingw1310_64/bin"
set "NINJA=D:/Qt/Tools/Ninja/ninja.exe"

set "PATH=%MINGW%;%QT_DIR%/bin;%NINJA%;%PATH%"

where gcc   || echo ERROR: gcc not found
where g++   || echo ERROR: g++ not found
where ninja || echo ERROR: ninja not found

if not exist build mkdir build
cd build

echo.
echo Configuring...
"%CMAKE%" .. -G Ninja -DCMAKE_PREFIX_PATH=%QT_DIR% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=%MINGW%/gcc.exe -DCMAKE_CXX_COMPILER=%MINGW%/g++.exe -DCMAKE_MAKE_PROGRAM=%NINJA%

if errorlevel 1 (
    echo Configure failed
    cd ..
    pause
    exit /b 1
)

echo.
echo Building...
"%CMAKE%" --build . --parallel

if errorlevel 1 (
    echo Build failed
) else (
    echo Done. Run: build\QTxLights.exe
)

cd ..
pause