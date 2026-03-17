#include "obj_output.hpp"

#include <fstream>

// Implementasi fungsi untuk menulis seluruh vertex dan face ke file OBJ
bool OBJOutput::writeOBJ(
    const std::string& outputPath,
    const std::vector<Vertex>& vertices,
    const std::vector<Face>& faces) {
    std::ofstream out(outputPath);
    if (!out.is_open()) return false;

    out << "# Processed OBJ output\n";
    out << "# Vertices: " << vertices.size() << "\n";
    out << "# Faces: " << faces.size() << "\n\n";

    for (int i = 0; i < (int)(vertices.size()); i++) {
        const Vertex& v = vertices[i];
        out << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }
    out << "\n";

    for (int i = 0; i < (int)(faces.size()); i++) {
        const Face& f = faces[i];
        out << "f " << (f.v1 + 1) << " " << (f.v2 + 1) << " " << (f.v3 + 1) << "\n";
    }

    return true;
}
