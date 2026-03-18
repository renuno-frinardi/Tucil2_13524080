#include <iostream>
#include <cstdlib>

#include "obj_handler/obj_parser.hpp"
#include "obj_handler/obj_output.hpp"
#include "voxelizer/octree.hpp"

int main(int argc, char* argv[]) {
    std::string filepath = argv[1];
    int depth = std::atoi(argv[2]);
    std::string outputPath = argv[3];

    OBJ obj;

    if (!obj.loadOBJ(filepath)) {
        printf("[ERROR] Gagal load OBJ\n");
        return 1;
    }

    printf("[LOG] OBJ berhasil dimuat!\n");

    Octree tree(obj.getVertices(), obj.getFaces(), depth);
    tree.build();

    printf("[LOG] Octree selesai!\n");

    std::vector<Vertex> voxelVertices;
    std::vector<Face> voxelFaces;
    tree.generateVoxelMesh(voxelVertices, voxelFaces);

    printf("[LOG] Voxel count: %lu\n", tree.getTotalVoxels());
    printf("[LOG] Vertex count: %lu\n", voxelVertices.size());
    printf("[LOG] Face count: %lu\n", voxelFaces.size());
    printf("[LOG] Depth: %d\n", depth);
    if (tree.getTimeTakenMs() >= 1000) printf("[LOG] Waktu yang dibutuhkan: %d detik\n", tree.getTimeTakenMs() / 1000);
    else printf("[LOG] Waktu yang dibutuhkan: %d milidetik\n", tree.getTimeTakenMs());

    for (int i = 1; i <= depth; i++) {
        printf("[LOG] %d: Terbentuk %d node\n", i, tree.getNodeCountAtDepth(i));
    };
    for (int i = 1; i <= depth; i++) {
        printf("[LOG] %d: %d node tidak ditelusuri\n", i, tree.getLeafCountAtDepth(i));
    };
    printf("\n");

    if (!OBJOutput::writeOBJ(outputPath, voxelVertices, voxelFaces)) {
        printf("[ERROR] Gagal menulis file output OBJ: %s\n", outputPath.c_str());
        return 1;
    }

    printf("[LOG] Output OBJ berhasil ditulis ke: %s\n", outputPath.c_str());

    return 0;
}