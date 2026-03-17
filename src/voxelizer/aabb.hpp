#pragma once

#include "geometry.h"
#include <algorithm>

// Struktur untuk menyimpan informasi Axis-Aligned Bounding Box (AABB)
struct AABB {
    Vertex min;
    Vertex max;

    // Inisiasi konstruktor dari AABB
    AABB() {}
    AABB(const Vertex& min, const Vertex& max) : min(min), max(max) {}

    // Fungsi untuk menghitung pusat dari AABB
    Vertex center() const {
        return {
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f,
            (min.z + max.z) * 0.5f
        };
    }

    // Fungsi untuk mengecek keteririsan antara dua AABB
    bool intersects(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
               (min.y <= other.max.y && max.y >= other.min.y) &&
               (min.z <= other.max.z && max.z >= other.min.z);
    }
};