#pragma once

#include <vector>
#include <iostream>
#include "aabb.hpp"

// Kelas untuk node dari struktur Octree
class OctreeNode {
    private:
        // Properti yang dimiliki sebuah OctreeNode
        AABB boundary;
        std::vector<int> triangleIndices;
        OctreeNode* children[8];
        bool isLeaf;

    public:
        // Konstruktor dan destruktor dari OctreeNode
        OctreeNode(const AABB& boundary) {
        for (int i = 0; i < 8; i++) setChild(i, nullptr);
            if (!setBoundary(boundary) || !setIsLeaf(true) || !setTriangles({})) {
                printf("Error dalam inisasi AABB\n");
            }
        }
        ~OctreeNode() {
            for (int i = 0; i < 8; i++) {
                if (getChild(i) != nullptr) {
                    delete children[i];
                }
            }
        }

        // === Getter dan Setter untuk properti dari OctreeNode === //

        // Setter dan getter untuk triangleIndices
        bool setTriangles(const std::vector<int>& indices) {
            triangleIndices = indices;
            return true;
        }
        const std::vector<int>& getTriangles() const {
            return triangleIndices;
        }

        // Setter dan getter untuk boundary
        bool setBoundary(const AABB& newBoundary) {
            boundary = newBoundary;
            return true;
        }
        const AABB& getBoundary() const {
            return boundary;
        }

        // Setter dan getter untuk isLeaf
        bool setIsLeaf(bool val) {
            isLeaf = val;
            return true;
        }
        bool getIsLeaf() const {
            return isLeaf;
        }

        // Setter dan getter untuk children
        bool setChild(int i, OctreeNode* node) {
            children[i] = node;
            return true;
        }
        OctreeNode* getChild(int i) const {
            return children[i];
        }
};