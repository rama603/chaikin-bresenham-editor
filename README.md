# PolySculpt: Interactive 2D Graphics & 3D Mesh Subdivision Studio

A custom 2D vector graphics editor and 3D mesh processing studio built from scratch in C++ using the **SFML** graphics library. This project implements mathematical algorithms from scratch, including custom rasterization, procedural curve smoothing, and 3D topological subdivision.

---

## Features

### 1. 2D Vector Graphics & Curve Editor
* **Custom Rasterization:** Draws lines and elements using a hand-implemented Bresenham's Line Algorithm directly onto a pixel buffer.
* **Curve Smoothing (Chaikin's Algorithm):** Dynamically smooths polygon control meshes with adjustable subdivision levels (up to level 5).
* **Multi-Shape Support & 2D HSR:** Draw, edit, and manage multiple independent shapes with a 2D Painter's Algorithm for hidden surface removal and opaque filling.
* **Precise Line-Segment Selection:** Employs point-to-segment distance checking to select and re-edit overlapping shapes cleanly by clicking directly on their lines.
* **Open Paths & Closed Loops:** Easily toggle between open paths and closed-loop filled shapes (`'O'` key).
* **Interactive Control Mesh:** Drag individual control points to manipulate shapes in real time, with a toggle for original mesh visibility (`'H'` key).

### 2. 3D Mesh Smoothing & Subdivision Studio
* **Multiple Primitives:** Toggle seamlessly between a **3D Cone** and a **3D Cube** primitive.
* **Doo-Sabin Topological Subdivision:** Subdivides quad meshes to mathematically smooth sharp corners, transforming blocky shapes (like a cube) into smooth limit spheres.
* **Laplacian Smoothing:** Relaxes vertex positions toward local neighbors for surface smoothing.
* **Custom 3D Engine:** Implements manual rotation controls, perspective projection, and a depth-sorted 3D Painter's Algorithm for hidden surface removal.

---

## Project Architecture

The codebase is organized into modular files:
* `Mesh3D.hpp` — Contains shared 3D data structures (`Vector3`, `Triangle3D`, `Face`).
* `ConeMesh3D.hpp` — Handles the cone geometry, grid-based topological subdivisions, and smoothing.
* `CubeMesh3D.hpp` — Manages the cube geometry, watertight face splits, and spherification.
* `main.cpp` — Manages the SFML render window, event loops, 2D editing canvas, and UI overlay.

---

## Controls & Shortcuts

### General & View Modes
* **`3` or Numpad `3`** : Toggle between the 2D Editor and the 3D Mesh Smoothing Mode.
* **`Esc`** : Close instruction help modal.

### 2D Mode Shortcuts
* **Left-Click** : Add control point / Click shape line to select & re-edit.
* **Drag Vertex** : Reshape the active control polygon.
* **`S` Key** : Increase Chaikin subdivision smoothing level.
* **`R` Key** : Decrease 2D smoothing level.
* **`O` Key** : Toggle between Open Path and Closed Loop mode.
* **`F` Key** : Finish current shape (locks it and de-selects).
* **`H` Key** : Toggle original control mesh visibility (Visible / Hidden).
* **`C` Key** : Clear all shapes from the canvas.

### 3D Mode Shortcuts
* **`S` Key** : Apply active 3D smoothing algorithm (Laplacian or Doo-Sabin) and subdivide.
* **`R` Key** : Reset the 3D mesh back to its original base resolution and geometry.
* **Mouse Drag** : Orbit/rotate around the 3D object.

---

## Getting Started & Building

### Prerequisites
* A C++ compiler supporting C++17 or later.
* SFML installed and configured for your development environment.

### Compilation
Make sure you link the SFML graphics, window, and system libraries, and include all source files (`main.cpp`, alongside your header files). If using CMake:
```bash
cmake -S . -B build
cmake --build build