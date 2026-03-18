@echo off
setlocal enabledelayedexpansion

echo Compiling Voxelizer (CLI)...
echo Platform: Windows

if not exist bin mkdir bin

echo Compiling source files...
g++ -Isrc ^
    src\main.cpp ^
    src\obj_handler\obj_parser.cpp ^
    src\obj_handler\obj_output.cpp ^
    src\voxelizer\octree.cpp ^
    -o bin\voxelizer.exe ^
    -std=c++17 ^
    -pthread ^
    -Wall -Wextra

if %errorlevel%==0 (
    echo Build success! Binary Location: bin/voxelizer.exe
) else (
    echo Build failed!
    pause
    exit /b 1
)
pause