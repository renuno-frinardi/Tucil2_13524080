#!/bin/bash

echo "Compiling Voxelizer..."

mkdir -p bin
g++ src/*.cpp -o bin/voxelizer -std=c++17 -pthread

if [ $? -eq 0 ]; then
    echo "Build success!"
    
else
    echo "Build failed!"
fi