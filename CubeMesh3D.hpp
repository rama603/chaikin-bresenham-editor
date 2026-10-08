#pragma once
#include "Mesh3D.hpp"
#include <cmath>
#include <algorithm>

class CubeMesh3D {
public:
    float size = 100.f;
    std::vector<Vector3> vertices;
    std::vector<Face> faces;
    std::vector<Triangle3D> triangles;

    CubeMesh3D() {
        generateBaseMesh();
    }

    void generateBaseMesh() {
        vertices.clear();
        faces.clear();
        triangles.clear();

        float s = size / 2.0f;
        vertices = {
            {-s, -s, -s}, { s, -s, -s}, { s,  s, -s}, {-s,  s, -s},
            {-s, -s,  s}, { s, -s,  s}, { s,  s,  s}, {-s,  s,  s}
        };

        faces = {
            {{0, 1, 2, 3}},
            {{5, 4, 7, 6}},
            {{4, 0, 3, 7}},
            {{1, 5, 6, 2}},
            {{4, 5, 1, 0}},
            {{3, 2, 6, 7}}
        };

        rebuildTrianglesFromFaces();
    }

    void applyLaplacianSmoothing() {
        const int neighbors[8][3] = {
            {1, 3, 4}, {0, 2, 5}, {1, 3, 6}, {0, 2, 7},
            {0, 5, 7}, {1, 4, 6}, {2, 5, 7}, {3, 4, 6}
        };

        std::vector<Vector3> newVertices = vertices;
        for (int i = 0; i < std::min(8, static_cast<int>(vertices.size())); ++i) {
            Vector3 sum = vertices[neighbors[i][0]] + vertices[neighbors[i][1]] + vertices[neighbors[i][2]];
            newVertices[i] = (sum + vertices[i] * 2.0f) / 5.0f;
        }
        vertices = newVertices;
        rebuildTrianglesFromFaces();
    }

    void applyDooSabin() {
        if (vertices.size() > 500) return;

        std::vector<Vector3> newVertices = vertices;
        std::vector<Face> newFaces;

        auto getMidpoint = [&](int i1, int i2) {
            Vector3 m = (vertices[i1] + vertices[i2]) * 0.5f;
            float len = std::sqrt(m.x * m.x + m.y * m.y + m.z * m.z);
            if (len > 0.01f) {
                float targetRadius = size * 0.866f;
                m = m * (targetRadius / len);
            }
            newVertices.push_back(m);
            return static_cast<int>(newVertices.size() - 1);
        };

        for (const auto& face : faces) {
            if (face.vertexIndices.size() != 4) continue;
            int v0 = face.vertexIndices[0];
            int v1 = face.vertexIndices[1];
            int v2 = face.vertexIndices[2];
            int v3 = face.vertexIndices[3];

            Vector3 centroid = (vertices[v0] + vertices[v1] + vertices[v2] + vertices[v3]) * 0.25f;
            float len = std::sqrt(centroid.x * centroid.x + centroid.y * centroid.y + centroid.z * centroid.z);
            if (len > 0.01f) {
                float targetRadius = size * 0.7f;
                centroid = centroid * (targetRadius / len);
            }
            newVertices.push_back(centroid);
            int cIdx = static_cast<int>(newVertices.size() - 1);

            int e0 = getMidpoint(v0, v1);
            int e1 = getMidpoint(v1, v2);
            int e2 = getMidpoint(v2, v3);
            int e3 = getMidpoint(v3, v0);

            newFaces.push_back({{v0, e0, cIdx, e3}});
            newFaces.push_back({{e0, v1, e1, cIdx}});
            newFaces.push_back({{cIdx, e1, v2, e2}});
            newFaces.push_back({{e3, cIdx, e2, v3}});
        }

        faces = newFaces;
        vertices = newVertices;
        rebuildTrianglesFromFaces();
    }

    void rebuildTrianglesFromFaces() {
        triangles.clear();
        for (size_t fIdx = 0; fIdx < faces.size(); ++fIdx) {
            const auto& face = faces[fIdx];
            int n = static_cast<int>(face.vertexIndices.size());
            if (n < 3) continue;

            sf::Color faceColor = sf::Color(100, 150 + (fIdx % 6) * 15, 200);

            int p0 = face.vertexIndices[0];
            for (int i = 1; i < n - 1; ++i) {
                int p1 = face.vertexIndices[i];
                int p2 = face.vertexIndices[i + 1];
                triangles.push_back({p0, p1, p2, faceColor, 0.f});
            }
        }
    }
};