@echo off
REM Build Dots and Boxes on Windows (requires CMake, a C++23 compiler, and Qt6).
REM Usage: run this file from the repository root.

REM 1. Fetch dependencies (Intel TBB, thread-safe LRU cache)
git submodule update --init --recursive
if errorlevel 1 (
    echo [ERROR] Failed to fetch submodules.
    exit /b 1
)

REM 2. Configure
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo [ERROR] CMake configure failed.
    exit /b 1
)

REM 3. Build
cmake --build build --config Release --parallel
if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

echo.
echo Build succeeded. The executable is at build\Release\Dots_and_Boxes.exe
echo To make a standalone distributable, run:  windeployqt build\Release\Dots_and_Boxes.exe
