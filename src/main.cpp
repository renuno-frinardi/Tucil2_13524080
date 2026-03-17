#include <iostream>
#include "obj_handler/obj_parser.hpp"

int main(int argc, char* argv[]) {
    OBJParser obj;

    if (obj.loadOBJ(argv[1])) {
        printf("File OBJ berhasil dimuat\n");
        printf("Vertices:\n");
        for (const auto& vertex : obj.getVertices()) {
            printf("x: %f, y: %f, z: %f\n", vertex.x, vertex.y, vertex.z);
        }

        printf("\nFaces:\n");
        for (const auto& face : obj.getFaces()) {
            printf("v1: %d, v2: %d, v3: %d\n", face.v1, face.v2, face.v3);
        }
        
        printf("Total Faces: %lu\n", obj.getFaces().size());
        printf("Total Vertices: %lu\n", obj.getVertices().size());
    } 
    
    else {
        printf("Gagal memuat file OBJ.\n");
    }
    return 0;
}