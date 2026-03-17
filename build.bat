@echo off
echo Compiling Voxelizer...

if not exist bin mkdir bin
g++ src\main.cpp src\obj_handler\obj_parser.cpp -o bin\voxelizer.exe -std=c++17 -pthread

if %errorlevel%==0 (
    echo Build success!
) else (
    echo Build failed!
)
pause