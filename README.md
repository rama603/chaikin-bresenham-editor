# Chaikin & Bresenham Multi-Shape Editor

A custom 2D vector graphics and curve-smoothing editor built in **C++** using the **SFML** graphics library. This project implements mathematical algorithms from scratch, including **Bresenham's Line Algorithm** for rasterization and **Chaikin's Corner-Cutting Algorithm** for curve smoothing.

---

##  Features

- **Custom Rasterization:** Draws all lines using a hand implemented Bresenham's Line Algorithm directly onto a pixel buffer.
- **Curve Smoothing (Chaikin's Algorithm):** Dynamically smooths polygon control meshes with adjustable subdivision levels (up to level 5).
- **Multi-Shape Support:** Draw, edit, and manage multiple independent shapes on the same canvas.
- **Precise Line-Segment Selection:** Employs point-to-segment distance checking so you can select and re-edit overlapping shapes cleanly by clicking directly on their lines.
- **Open Paths & Closed Loops:** Easily toggle between open paths and closed loops.
- **Interactive Control Mesh:** Drag individual control points to manipulate shapes in real time. Toggle the original mesh visibility on/off.

---

##  Controls & Shortcuts

- **Left-Click** | Add control point / Click shape line to select & re-edit 
- **Drag Vertex** | Reshape the active control polygon 
- **`S` Key** | Increase Chaikin subdivision smoothing level 
- **`R` Key** | Decrease smoothing level 
- **`O` Key** | Toggle between Open Path and Closed Loop mode 
- **`F` Key** | Finish current shape (locks it and de-selects) 
- **`H` Key** | Toggle original control mesh visibility (Visible / Hidden) 
- **`C` Key** | Clear all shapes from the canvas 


---

##  Getting Started & Building

### Prerequisites
- A C++ compiler supporting C++17 or later.
- **SFML** installed and configured for your development environment.

### Compilation 
Make sure you link the SFML graphics, window, and system libraries.
