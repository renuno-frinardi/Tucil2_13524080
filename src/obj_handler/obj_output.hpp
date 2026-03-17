#pragma once

#include <string>
#include <vector>

#include "geometry.h"

// Kelas untuk menghandle output ke file OBJ
class OBJOutput {
    public:
        static bool writeOBJ(const std::string& outputPath, const std::vector<Vertex>& vertices, const std::vector<Face>& faces);
};
