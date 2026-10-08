#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct Vector3 {
    float x, y, z;
    Vector3 operator+(const Vector3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vector3 operator-(const Vector3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vector3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vector3 operator/(float s) const { return {x / s, y / s, z / s}; }
};

struct Triangle3D {
    int v0, v1, v2;
    sf::Color color;
    float avgZ;
};

struct Face {
    std::vector<int> vertexIndices;
};