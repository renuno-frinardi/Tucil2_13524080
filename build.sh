#!/bin/bash

set -e

echo "Compiling Voxelizer (CLI)..."
echo "Platform: Linux/Unix"

mkdir -p bin

echo "Compiling source files..."
g++ -Isrc \
    src/main.cpp \
    src/obj_handler/obj_parser.cpp \
    src/obj_handler/obj_output.cpp \
    src/voxelizer/octree.cpp \
    -o bin/voxelizer \
    -std=c++17 \
    -pthread \
    -Wall -Wextra

echo "Build success! Binary Location: bin/voxelizer"