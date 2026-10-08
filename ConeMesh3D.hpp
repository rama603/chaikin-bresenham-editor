#pragma once
#include "Mesh3D.hpp"
#include <cmath>
#include <algorithm>

class ConeMesh3D {
public:
    int stacks = 12;
    int sectors = 16;
    float radius = 100.f;
    float height = 200.f;

    std::vector<Vector3> vertices;
    std::vector<std::vector<int>> gridIndices; 
    std::vector<Triangle3D> triangles;
    std::vector<Face> faces;

    ConeMesh3D() {
        generateBaseMesh();
    }

    void generateBaseMesh() {
        vertices.clear();
        gridIndices.assign(stacks + 1, std::vector<int>(sectors, -1));
        triangles.clear();
        faces.clear();

        for (int i = 0; i <= stacks; ++i) {
            float vFraction = static_cast<float>(i) / stacks; 
            float currentY = -height / 2.0f + vFraction * height;
            float currentRadius = radius * (1.0f - vFraction);

            for (int j = 0; j < sectors; ++j) {
                float uFraction = static_cast<float>(j) / sectors;
                float angle = uFraction * 2.0f * 3.14159265f;

                float x = currentRadius * std::cos(angle);
                float z = currentRadius * std::sin(angle);

                gridIndices[i][j] = static_cast<int>(vertices.size());
                vertices.push_back({x, currentY, z});
            }
        }

        for (int i = 0; i < stacks; ++i) {
            for (int j = 0; j < sectors; ++j) {
                int nextJ = (j + 1) % sectors;
                int p0 = gridIndices[i][j];
                int p1 = gridIndices[i][nextJ];
                int p2 = gridIndices[i + 1][nextJ];
                int p3 = gridIndices[i + 1][j];

                faces.push_back({{p0, p1, p2, p3}});
            }
        }

        vertices.push_back({0.f, -height / 2.0f, 0.f});
        std::vector<int> baseIndices;
        for (int j = 0; j < sectors; ++j) {
            baseIndices.push_back(gridIndices[0][j]);
        }
        faces.push_back({baseIndices});
        rebuildTrianglesFromFaces();
    }

    void applyLaplacianSmoothing() {
        std::vector<Vector3> newVertices = vertices;
        for (int i = 1; i < stacks; ++i) { 
            for (int j = 0; j < sectors; ++j) {
                int idx = gridIndices[i][j];
                if (idx < 0 || idx >= static_cast<int>(vertices.size())) continue;

                int left = gridIndices[i][(j - 1 + sectors) % sectors];
                int right = gridIndices[i][(j + 1) % sectors];
                int down = gridIndices[i - 1][j];
                int up = gridIndices[i + 1][j];

                if (left >= 0 && right >= 0 && down >= 0 && up >= 0 && 
                    left < vertices.size() && right < vertices.size() && down < vertices.size() && up < vertices.size()) {
                    Vector3 sum = vertices[left] + vertices[right] + vertices[down] + vertices[up];
                    newVertices[idx] = (sum + vertices[idx] * 2.0f) / 6.0f;
                }
            }
        }
        vertices = newVertices;
        rebuildTrianglesFromFaces();
    }

    void applyDooSabin() {
        if (stacks >= 128 || sectors >= 128) return;

        int oldStacks = stacks;
        int oldSectors = sectors;
        std::vector<Vector3> oldVertices = vertices;
        std::vector<std::vector<int>> oldGrid = gridIndices;

        stacks *= 2;
        sectors *= 2;

        vertices.clear();
        gridIndices.assign(stacks + 1, std::vector<int>(sectors, -1));
        faces.clear();

        for (int i = 0; i <= stacks; ++i) {
            float vFraction = static_cast<float>(i) / stacks; 
            float currentY = -height / 2.0f + vFraction * height;
            float currentRadius = radius * (1.0f - vFraction);

            for (int j = 0; j < sectors; ++j) {
                float uFraction = static_cast<float>(j) / sectors;
                float angle = uFraction * 2.0f * 3.14159265f;

                int oI = std::min(i / 2, oldStacks);
                int oJ = j / 2;
                Vector3 baseV = oldVertices[oldGrid[oI][oJ % oldSectors]];

                float x = currentRadius * std::cos(angle);
                float z = currentRadius * std::sin(angle);

                Vector3 smoothedV = {
                    (baseV.x + x) * 0.5f,
                    (baseV.y + currentY) * 0.5f,
                    (baseV.z + z) * 0.5f
                };

                gridIndices[i][j] = static_cast<int>(vertices.size());
                vertices.push_back(smoothedV);
            }
        }

        for (int i = 0; i < stacks; ++i) {
            for (int j = 0; j < sectors; ++j) {
                int nextJ = (j + 1) % sectors;
                int p0 = gridIndices[i][j];
                int p1 = gridIndices[i][nextJ];
                int p2 = gridIndices[i + 1][nextJ];
                int p3 = gridIndices[i + 1][j];

                faces.push_back({{p0, p1, p2, p3}});
            }
        }

        rebuildTrianglesFromFaces();
    }

    void rebuildTrianglesFromFaces() {
        triangles.clear();
        for (size_t fIdx = 0; fIdx < faces.size(); ++fIdx) {
            const auto& face = faces[fIdx];
            int n = static_cast<int>(face.vertexIndices.size());
            if (n < 3) continue;

            sf::Color faceColor = sf::Color(50, 120 + (fIdx % 8) * 15, 200);

            int p0 = face.vertexIndices[0];
            for (int i = 1; i < n - 1; ++i) {
                int p1 = face.vertexIndices[i];
                int p2 = face.vertexIndices[i + 1];
                triangles.push_back({p0, p1, p2, faceColor, 0.f});
            }
        }
    }
};