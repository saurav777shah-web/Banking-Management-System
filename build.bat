@echo off
setlocal

set BUILD_DIR=build

if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
if %ERRORLEVEL% neq 0 (echo CMake configure failed & exit /b 1)

mingw32-make -j%NUMBER_OF_PROCESSORS%
if %ERRORLEVEL% neq 0 (echo Build failed & exit /b 1)

echo.
echo Build successful! Run: build\banking_system.exe
`