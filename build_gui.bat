@echo off
setlocal enabledelayedexpansion

echo Compiling Voxelizer GUI (Raylib)...
echo Platform: Windows

if not exist bin mkdir bin

set RAYLIB_DIR=src\gui\raylib
set RAYLIB_SRC=%RAYLIB_DIR%\src

if not exist %RAYLIB_SRC%\raylib.h (
    echo Raylib source not found, cloning...
    git clone --depth 1 https://github.com/raysan5/raylib.git %RAYLIB_DIR%
    if !errorlevel! neq 0 goto :fail
)

echo Building Raylib (PLATFORM_DESKTOP)...
where mingw32-make >nul 2>nul
if !errorlevel!==0 (
    mingw32-make -C %RAYLIB_SRC% PLATFORM=PLATFORM_DESKTOP
) else (
    make -C %RAYLIB_SRC% PLATFORM=PLATFORM_DESKTOP
)
if !errorlevel! neq 0 goto :fail

echo Compiling GUI application...
g++ -Isrc -I%RAYLIB_SRC% ^
    src\gui\main_gui.cpp ^
    src\obj_handler\obj_parser.cpp ^
    src\obj_handler\obj_output.cpp ^
    src\voxelizer\octree.cpp ^
    -o bin\voxelizer_gui.exe ^
    -std=c++17 ^
    -pthread ^
    -Wall -Wextra ^
    -L%RAYLIB_SRC% ^
    -lraylib ^
    -lopengl32 ^
    -lgdi32 ^
    -lwinmm
if !errorlevel! neq 0 goto :fail

echo Build GUI success! Binary Location: bin/voxelizer_gui.exe
goto :done

:fail
echo Build GUI failed!
endlocal
pause
exit /b 1

:done
endlocal
pause
