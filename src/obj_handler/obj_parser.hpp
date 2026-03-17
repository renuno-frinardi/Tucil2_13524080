#pragma once

#include <vector>
#include <string>

// Struktur untuk menyimpan Vertex dalam satu OBJ file
struct Vertex {
    float x, y, z;
};

// Struktur untuk menyimpan face dalam satu OBJ file
struct Face {
    int v1, v2, v3;
};

// Kelas untuk processing file OBJ
class OBJParser {
    private:
        std::vector<Vertex> vertices;
        std::vector<Face> faces;

        int parseFaceVertexIndex(const std::string & token);
        bool parseVertex(const std::string& line);
        bool parseFace(const std::string& line);

    public:
        bool loadOBJ(const std::string& filename);

        const std::vector<Vertex>& getVertices() const;
        const std::vector<Face>& getFaces() const;
};