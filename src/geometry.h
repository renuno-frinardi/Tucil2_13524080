#pragma once

// Struktur untuk menyimpan Vertex dalam satu OBJ file
struct Vertex {
    float x, y, z;
};

// Struktur untuk menyimpan face dalam satu OBJ file
struct Face {
    int v1, v2, v3;
};
