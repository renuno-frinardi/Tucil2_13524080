#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <future>
#include <mutex>
#include <set>
#include "octree.hpp"

// Helper untuk MutEx lock dan grid cell
static std::mutex gDepthStatsMutex;
using GridCellKey = std::array<int, 3>;
static int coordToGridIndex(float value, float minValue, float step) {
    if (step == 0.0f) return 0;
    return (int)(std::lround((value - minValue) / step));
}

// Konstruktor dari Octree
Octree::Octree(const std::vector<Vertex> &v, const std::vector<Face> &f, int maxDepth)
    : root(nullptr), maxDepth(maxDepth), vertices(v), faces(f) {
    if (vertices.empty()) {
        root = new OctreeNode(AABB(Vertex{0.0f, 0.0f, 0.0f}, Vertex{0.0f, 0.0f, 0.0f}));
        return;
    }

    Vertex min = vertices[0];
    Vertex max = vertices[0];

    for (size_t i = 0; i < vertices.size(); i++) {
        const Vertex &vert = vertices[i];
        min.x = std::min(min.x, vert.x);
        min.y = std::min(min.y, vert.y);
        min.z = std::min(min.z, vert.z);

        max.x = std::max(max.x, vert.x);
        max.y = std::max(max.y, vert.y);
        max.z = std::max(max.z, vert.z);
    }

    for (int i = 0; i <= maxDepth; i++) {
        nodePerDepth.push_back(0);
        leafPerDepth.push_back(0);
    }

    root = new OctreeNode(AABB(min, max));

    std::vector<int> allFaces;
    allFaces.reserve(faces.size());
    for (int i = 0; i < (int)(faces.size()); i++) {
        allFaces.push_back(i);
    }

    root->setTriangles(allFaces);
}

// Destruktor dari Octree
Octree::~Octree() { delete root; }

// Inisiasi proses subdivisi dari Octree
void Octree::build() { subdivide(root, 0); }

// Fungsi untuk melakukan subdivisi pada node dari Octree
void Octree::subdivide(OctreeNode *node, int depth) {
    if (!node) return;

    {
        std::lock_guard<std::mutex> lock(gDepthStatsMutex);
        if (depth >= 0 && depth < (int)(nodePerDepth.size())) nodePerDepth[depth]++;
    }

    if (depth >= maxDepth || node->getTriangles().empty()) {
        std::lock_guard<std::mutex> lock(gDepthStatsMutex);
        if (depth >= 0 && depth < (int)(leafPerDepth.size())) leafPerDepth[depth]++;
        return;
    }

    std::vector<AABB> boxes = split(node->getBoundary());

    std::vector<std::future<void>> futures;
    bool hasChild = false;

    for (int i = 0; i < 8; i++) {
        std::vector<int> childFaces;

        for (size_t j = 0; j < node->getTriangles().size(); j++) {
            int idx = node->getTriangles()[j];
            if (intersects(idx, boxes[i])) childFaces.push_back(idx);
        }

        if (childFaces.empty()) {
            std::lock_guard<std::mutex> lock(gDepthStatsMutex);
            if (depth >= 0 && depth < (int)(leafPerDepth.size())) {
                leafPerDepth[depth]++;
            }
            continue;
        }
        
        OctreeNode *child = new OctreeNode(boxes[i]);
        child->setTriangles(childFaces);

        node->setChild(i, child);
        node->setIsLeaf(false);
        hasChild = true;

        if (depth < 2) futures.push_back(std::async(std::launch::async, [this, child, depth]() { subdivide(child, depth + 1); }));
        else subdivide(child, depth + 1);
    }

    for (size_t i = 0; i < futures.size(); i++) futures[i].get();

    if (!hasChild) {
        std::lock_guard<std::mutex> lock(gDepthStatsMutex);
        if (depth >= 0 && depth < (int)(leafPerDepth.size())) leafPerDepth[depth]++;
        return;
    }
}

// Helper untuk membagi AABB menjadi 8 sub bagian
std::vector<AABB> Octree::split(const AABB &b) {
    std::vector<AABB> result(8);
    Vertex m = b.center();

    for (int i = 0; i < 8; i++) {
        bool highX = (i & 4) != 0;
        bool highY = (i & 2) != 0;
        bool highZ = (i & 1) != 0;

        Vertex min;
        Vertex max;

        if (highX) {
            min.x = m.x;
            max.x = b.max.x;
        } else {
            min.x = b.min.x;
            max.x = m.x;
        }

        if (highY) {
            min.y = m.y;
            max.y = b.max.y;
        } else {
            min.y = b.min.y;
            max.y = m.y;
        }

        if (highZ) {
            min.z = m.z;
            max.z = b.max.z;
        } else {
            min.z = b.min.z;
            max.z = m.z;
        }

        result[i] = AABB(min, max);
    }

    return result;
}

// Helper untuk mengecek apakah sebuah face beririsan dengan AABB dari node Octree
bool Octree::intersects(int faceIdx, const AABB &box) {
    const Face &f = faces[faceIdx];

    Vertex a = vertices[f.v1];
    Vertex b = vertices[f.v2];
    Vertex c = vertices[f.v3];

    Vertex center = {
        (a.x + b.x + c.x) / 3.0f, 
        (a.y + b.y + c.y) / 3.0f,     
        (a.z + b.z + c.z) / 3.0f
    };

    if (center.x >= box.min.x && center.x <= box.max.x &&
        center.y >= box.min.y && center.y <= box.max.y &&
        center.z >= box.min.z && center.z <= box.max.z)
        return true;

    Vertex tmin = {
        std::min({a.x, b.x, c.x}), 
        std::min({a.y, b.y, c.y}),
        std::min({a.z, b.z, c.z})
    };

    Vertex tmax = {
        std::max({a.x, b.x, c.x}), 
        std::max({a.y, b.y, c.y}),
        std::max({a.z, b.z, c.z})
    };

    AABB triBox(tmin, tmax);

    return box.intersects(triBox);
}

// Helper untuk mengumpulkan leaf dari Octree
void Octree::collectLeaves(OctreeNode *node, std::vector<AABB> &boxes) const {
    if (!node) return;

    if (node->getIsLeaf()) {
        boxes.push_back(node->getBoundary());
        return;
    }

    for (int i = 0; i < 8; i++) collectLeaves(node->getChild(i), boxes);
}

// Method utama untuk mendapatkan leaf dari Octree
std::vector<AABB> Octree::getLeafBoxes() const {
    std::vector<AABB> boxes;
    collectLeaves(root, boxes);
    return boxes;
}

// Method untuk membuat mesh dari hasil vokselisasi
void Octree::generateVoxelMesh(std::vector<Vertex> &outVertices, std::vector<Face> &outFaces) const {
    std::vector<AABB> leaves = getLeafBoxes();
    printf("Total leaf nodes: %zu\n", leaves.size());

    if (leaves.empty()) return;

    const AABB &rootBox = root->getBoundary();
    int gridSize = 1 << std::max(0, maxDepth);
    if (gridSize <= 0)gridSize = 1;

    float stepX = (rootBox.max.x - rootBox.min.x) / (float)(gridSize);
    float stepY = (rootBox.max.y - rootBox.min.y) / (float)(gridSize);
    float stepZ = (rootBox.max.z - rootBox.min.z) / (float)(gridSize);

    std::vector<GridCellKey> leafKeys;
    leafKeys.reserve(leaves.size());

    std::set<GridCellKey> occupied;
    for (size_t i = 0; i < leaves.size(); i++) {
        GridCellKey key = {
            coordToGridIndex(leaves[i].min.x, rootBox.min.x, stepX),
            coordToGridIndex(leaves[i].min.y, rootBox.min.y, stepY),
            coordToGridIndex(leaves[i].min.z, rootBox.min.z, stepZ),
        };

        leafKeys.push_back(key);
        occupied.insert(key);
    }

    size_t surfaceVoxelCount = 0;

    for (size_t i = 0; i < leaves.size(); i++) {
        const GridCellKey &k = leafKeys[i];
        bool hasXp = occupied.count({k[0] + 1, k[1], k[2]}) > 0;
        bool hasXn = occupied.count({k[0] - 1, k[1], k[2]}) > 0;
        bool hasYp = occupied.count({k[0], k[1] + 1, k[2]}) > 0;
        bool hasYn = occupied.count({k[0], k[1] - 1, k[2]}) > 0;
        bool hasZp = occupied.count({k[0], k[1], k[2] + 1}) > 0;
        bool hasZn = occupied.count({k[0], k[1], k[2] - 1}) > 0;

        if (hasXp && hasXn && hasYp && hasYn && hasZp && hasZn) continue;

        const AABB &box = leaves[i];
        int base = (int)outVertices.size();
        surfaceVoxelCount++;

        float x0 = box.min.x, y0 = box.min.y, z0 = box.min.z;
        float x1 = box.max.x, y1 = box.max.y, z1 = box.max.z;

        outVertices.push_back({x0, y0, z0}); 
        outVertices.push_back({x1, y0, z0});
        outVertices.push_back({x1, y1, z0}); 
        outVertices.push_back({x0, y1, z0}); 
        outVertices.push_back({x0, y0, z1}); 
        outVertices.push_back({x1, y0, z1}); 
        outVertices.push_back({x1, y1, z1}); 
        outVertices.push_back({x0, y1, z1}); 
      
        if (!hasZp) {
            outFaces.push_back({base + 4, base + 5, base + 6});
            outFaces.push_back({base + 4, base + 6, base + 7});
        }
       
        if (!hasZn) {
            outFaces.push_back({base + 1, base + 0, base + 3});
            outFaces.push_back({base + 1, base + 3, base + 2});
        }

        if (!hasXp) {
            outFaces.push_back({base + 5, base + 1, base + 2});
            outFaces.push_back({base + 5, base + 2, base + 6});
        }

        if (!hasXn) {
            outFaces.push_back({base + 0, base + 4, base + 7});
            outFaces.push_back({base + 0, base + 7, base + 3});
        }

        if (!hasYp) {
            outFaces.push_back({base + 3, base + 7, base + 6});
            outFaces.push_back({base + 3, base + 6, base + 2});
        }

        if (!hasYn) {
            outFaces.push_back({base + 0, base + 1, base + 5});
            outFaces.push_back({base + 0, base + 5, base + 4});
        }
    }

}

// Getter untuk mendapatkan jumlah node pada kedalaman tertentu
int Octree::getNodeCountAtDepth(int depth) const {
    if (depth < 0 || depth >= (int)(nodePerDepth.size())) return 0;
    return nodePerDepth[depth];
}

// Getter untuk mendapatkan jumlah node yang tidak ditelusuri pada kedalaman tertentu
int Octree::getLeafCountAtDepth(int depth) const {
    if (depth < 0 || depth >= (int)(leafPerDepth.size())) return 0;
    return leafPerDepth[depth];
}

