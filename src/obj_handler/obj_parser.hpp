#pragma once

#include <vector>
#include <string>
#include "geometry.h"

// Kelas untuk processing file OBJ
class OBJ {
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