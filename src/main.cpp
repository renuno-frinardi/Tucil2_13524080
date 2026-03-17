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
        printf("Gagal load OBJ\n");
        return 1;
    }

    printf("OBJ Loaded!\n");
    printf("Vertices: %lu\n", obj.getVertices().size());
    printf("Faces: %lu\n", obj.getFaces().size());

    Octree tree(obj.getVertices(), obj.getFaces(), depth);
    tree.build();

    printf("Octree selesai!\n");

    std::vector<Vertex> voxelVertices;
    std::vector<Face> voxelFaces;
    tree.generateVoxelMesh(voxelVertices, voxelFaces);

    printf("Voxels: %lu\n", voxelVertices.size() / 8);
    printf("Banyak node yang ada ditiap kedalaman:\n");
    for (int i = 1; i <= depth; i++) {
        printf("Depth %d: %d Nodes\n", i, tree.getNodeCountAtDepth(i));
    };
    printf("Banyak node yang tidak ditelusuri ditiap kedalaman:\n");
    for (int i = 1; i <= depth; i++) {
        printf("Depth %d: %d Leaf Nodes\n", i, tree.getLeafCountAtDepth(i));
    };
    printf("\n");

    if (!OBJOutput::writeOBJ(outputPath, voxelVertices, voxelFaces)) {
        printf("Gagal menulis file output OBJ: %s\n", outputPath.c_str());
        return 1;
    }

    printf("Output OBJ berhasil ditulis ke: %s\n", outputPath.c_str());

    return 0;
}