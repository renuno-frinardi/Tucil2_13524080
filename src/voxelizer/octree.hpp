#pragma once

#include <vector>
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <future>
#include <mutex>
#include <set>
#include <chrono>
#include "geometry.h"
#include "aabb.hpp"
#include "octree_node.hpp"

// Kelas untuk struktur dari Octree
class Octree {
    private:
         // Properti yang dimiliki sebuah Octree
        OctreeNode* root;
        int maxDepth;
        mutable int totalVoxels;
        const std::vector<Vertex>& vertices;
        const std::vector<Face>& faces;
        std::vector<int> nodePerDepth;
        std::vector<int> leafPerDepth;
        std::chrono::steady_clock::time_point start;
        std::chrono::steady_clock::time_point end;
        std::chrono::milliseconds duration;

        // Helper untuk mengolah data OBJ dan proses vokseliasi
        void subdivide(OctreeNode* node, int depth);
        std::vector<AABB> split(const AABB& box);
        bool intersects(int faceIdx, const AABB& box);
        void collectLeaves(OctreeNode* node, std::vector<AABB>& boxes) const;

    public:
        // Konstruktor dan destruktor dari Octree
        Octree(const std::vector<Vertex>& v,
            const std::vector<Face>& f,
            int maxDepth);
        ~Octree();
        
        // Method utama untuk membuat Octree, mendapatkan node terakhir, dan melakukan generasi mesh akhir
        void build();
        std::vector<AABB> getLeafBoxes() const;
        void generateVoxelMesh(std::vector<Vertex>& outVertices, std::vector<Face>& outFaces) const;
        int getNodeCountAtDepth(int depth) const;
        int getLeafCountAtDepth(int depth) const;
        int getTotalVoxels() const { return totalVoxels; }
        int getTimeTakenMs() const;
};