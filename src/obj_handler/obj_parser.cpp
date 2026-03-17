#include "obj_parser.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

// Parser file OBJ untuk menyimpan Vertices dan Faces dalam suatu file OBJ
bool OBJParser::loadOBJ(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        printf("File tidak bisa dibuka: %s\n", filename.c_str());
        return false;
    }

    std::string line;
    int lineCount = 0;

    while (std::getline(file, line)) {
        lineCount++;

        if (line.empty() || line[0] == '#') continue;

        if (line.size() > 1 && line[0] == 'v' && line[1] == ' ') {
            if (!parseVertex(line)) {
                printf("Vertex tidak bisa dibaca pada baris %d: %s\n", lineCount, line.c_str());
            }
            continue;
        }

        if (line.size() > 1 && line[0] == 'f' && line[1] == ' ') {
            if (!parseFace(line)) {
                printf("Face tidak bisa dibaca pada baris %d: %s\n", lineCount, line.c_str());
            }
            continue;
        }
    }

    if (vertices.empty() || faces.empty()) {
        printf("File OBJ tidak valid: %s\n", filename.c_str());
        return false;
    }

    return true;
}

// Parser untuk menghandle Vertex dalam file OBJ
bool OBJParser::parseVertex(const std::string& line) {
    std::istringstream curLine(line);
    char prefix;
    float x, y, z;

    if (!(curLine >> prefix >> x >> y >> z)) return false;

    vertices.push_back({x, y, z});
    return true;
}

// Parser untuk menghandle Face dalam file OBJ
bool OBJParser::parseFace(const std::string& line) {
    std::istringstream curLine(line);
    char prefix;
    std::string t1, t2, t3;

    if (!(curLine >> prefix >> t1 >> t2 >> t3)) return false;

    int v1 = parseFaceVertexIndex(t1);
    int v2 = parseFaceVertexIndex(t2);
    int v3 = parseFaceVertexIndex(t3);

    if (v1 <= 0 || v2 <= 0 || v3 <= 0) return false;

    // OBJ vertex index dimulai dari 1, internal vector dari 0.
    v1--; v2--; v3--;

    if (v1 < 0 || v2 < 0 || v3 < 0 ||
        v1 >= static_cast<int>(vertices.size()) ||
        v2 >= static_cast<int>(vertices.size()) ||
        v3 >= static_cast<int>(vertices.size())) {
        return false;
    }

    faces.push_back({v1, v2, v3});
    return true;
}

// Parser untuk menghandle index vertex dalam face
int OBJParser::parseFaceVertexIndex(const std::string& token) {
    size_t slashPos = token.find('/');
    
    std::string indexPart;
    if (slashPos == std::string::npos) indexPart = token;
    else indexPart = token.substr(0, slashPos);

    if (indexPart.empty()) return -1;

    return std::stoi(indexPart);
}

// Getter dari vertices
const std::vector<Vertex>& OBJParser::getVertices() const { return vertices; }

// Getter dari faces
const std::vector<Face>& OBJParser::getFaces() const { return faces; }