#!/bin/bash
set -e

echo "Compiling Voxelizer GUI (Raylib)..."
echo "Platform: Linux/Unix"

mkdir -p bin

RAYLIB_DIR="src/gui/raylib"
RAYLIB_SRC="$RAYLIB_DIR/src"

if [ ! -f "$RAYLIB_SRC/raylib.h" ]; then
    echo "Raylib source tidak ditemukan, cloning..."
    git clone --depth 1 https://github.com/raysan5/raylib.git "$RAYLIB_DIR"
fi

echo "Building Raylib (PLATFORM_DESKTOP)..."
make -C "$RAYLIB_SRC" PLATFORM=PLATFORM_DESKTOP

echo "Compiling GUI application..."
g++ -Isrc -I"$RAYLIB_SRC" \
    src/gui/main_gui.cpp \
    src/obj_handler/obj_parser.cpp \
    src/obj_handler/obj_output.cpp \
    src/voxelizer/octree.cpp \
    -o bin/voxelizer_gui \
    -std=c++17 \
    -pthread \
    -Wall -Wextra \
    -L"$RAYLIB_SRC" \
    -lraylib \
    -lm \
    -lpthread \
    -ldl \
    -lX11

echo "Build GUI success! Binary Location: bin/voxelizer_gui"
