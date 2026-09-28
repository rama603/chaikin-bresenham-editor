#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

struct ShapeObject {
    std::vector<sf::Vector2f> controlPoints;
    int subdivisionLevel = 0;
    bool isClosedLoop = false;
    bool isFinished = false; // Finished objects are locked until selected again
};

// Custom Bresenham's Line Algorithm
void drawBresenhamLine(std::vector<std::uint8_t>& pixels, int width, int height, int x0, int y0, int x1, int y1, sf::Color color) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
            int index = (y0 * width + x0) * 4;
            pixels[index] = color.r;     
            pixels[index + 1] = color.g; 
            pixels[index + 2] = color.b; 
            pixels[index + 3] = color.a; 
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Calculates the shortest distance from point p to line segment ab (used for precise shape selection)
float pointToSegmentDistance(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b) {
    sf::Vector2f ab = b - a;
    sf::Vector2f ap = p - a;
    float abLenSq = ab.x * ab.x + ab.y * ab.y;
    
    if (abLenSq == 0.f) {
        return std::sqrt(ap.x * ap.x + ap.y * ap.y);
    }
    
    float t = (ap.x * ab.x + ap.y * ab.y) / abLenSq;
    t = std::max(0.0f, std::min(1.0f, t));
    
    sf::Vector2f projection = a + t * ab;
    sf::Vector2f diff = p - projection;
    return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}

// Chaikin's Subdivision Algorithm
std::vector<sf::Vector2f> chaikinSubdivide(const std::vector<sf::Vector2f>& points, bool isClosed) {
    if (points.size() < 2) return points;
    std::vector<sf::Vector2f> newPoints;
    size_t n = points.size();
    
    if (!isClosed) {
        newPoints.push_back(points[0]);
    }

    size_t limit = isClosed ? n : n - 1;
    for (size_t i = 0; i < limit; ++i) {
        sf::Vector2f p0 = points[i];
        sf::Vector2f p1 = points[(i + 1) % n]; 

        sf::Vector2f q = p0 * 0.75f + p1 * 0.25f;
        sf::Vector2f r = p0 * 0.25f + p1 * 0.75f;

        newPoints.push_back(q);
        newPoints.push_back(r);
    }

    if (!isClosed) {
        newPoints.push_back(points[n - 1]);
    }

    return newPoints;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Chaikin & Bresenham Multi-Shape Editor");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cerr << "Warning: Could not load system font from C:/Windows/Fonts/arial.ttf" << std::endl;
    }

    std::vector<ShapeObject> shapes;
    int activeShapeIndex = -1; // -1 means no active shape being edited/drawn

    bool showOriginalMesh = true; // Toggle via 'H' key
    bool showHelpModal = false;

    // Dragging state tracking for the active shape
    bool isDragging = false;
    int draggedPointIndex = -1;

    std::vector<std::uint8_t> pixelBuffer(WINDOW_WIDTH * WINDOW_HEIGHT * 4, 255);
    
    sf::Texture canvasTexture;
    if (!canvasTexture.resize({static_cast<unsigned int>(WINDOW_WIDTH), static_cast<unsigned int>(WINDOW_HEIGHT)})) {
        std::cerr << "Failed to create texture!" << std::endl;
        return -1;
    }
    sf::Sprite canvasSprite(canvasTexture);

    auto renderCanvas = [&]() {
        std::fill(pixelBuffer.begin(), pixelBuffer.end(), 255);

        for (size_t sIdx = 0; sIdx < shapes.size(); ++sIdx) {
            const auto& shape = shapes[sIdx];
            bool isActive = (static_cast<int>(sIdx) == activeShapeIndex);

            // 1. Draw Control Polygon lines (Gray) if enabled or if it's the active shape
            if ((showOriginalMesh || isActive) && shape.controlPoints.size() > 1) {
                size_t limit = shape.isClosedLoop ? shape.controlPoints.size() : shape.controlPoints.size() - 1;
                for (size_t i = 0; i < limit; ++i) {
                    sf::Vector2f p0 = shape.controlPoints[i];
                    sf::Vector2f p1 = shape.controlPoints[(i + 1) % shape.controlPoints.size()];
                    drawBresenhamLine(pixelBuffer, WINDOW_WIDTH, WINDOW_HEIGHT, 
                                      static_cast<int>(p0.x), static_cast<int>(p0.y), 
                                      static_cast<int>(p1.x), static_cast<int>(p1.y), 
                                      isActive ? sf::Color(150, 150, 150) : sf::Color(220, 220, 220));
                }
            }

            // 2. Compute and Draw Chaikin Smoothed Curve (Blue for active, Dark Gray for finished)
            if (shape.controlPoints.size() > 1) {
                std::vector<sf::Vector2f> currentPoints = shape.controlPoints;
                for (int lvl = 0; lvl < shape.subdivisionLevel; ++lvl) {
                    currentPoints = chaikinSubdivide(currentPoints, shape.isClosedLoop);
                }

                if (currentPoints.size() > 1) {
                    size_t limit = shape.isClosedLoop ? currentPoints.size() : currentPoints.size() - 1;
                    sf::Color curveColor = isActive ? sf::Color(0, 100, 255) : sf::Color(100, 100, 100);
                    for (size_t i = 0; i < limit; ++i) {
                        sf::Vector2f p0 = currentPoints[i];
                        sf::Vector2f p1 = currentPoints[(i + 1) % currentPoints.size()];
                        drawBresenhamLine(pixelBuffer, WINDOW_WIDTH, WINDOW_HEIGHT, 
                                          static_cast<int>(p0.x), static_cast<int>(p0.y), 
                                          static_cast<int>(p1.x), static_cast<int>(p1.y), 
                                          curveColor);
                    }
                }
            }
        }

        canvasTexture.update(pixelBuffer.data());
    };

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // Mouse Button Pressed
            if (const auto* mousePress = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePress->button == sf::Mouse::Button::Left) {
                    float mx = static_cast<float>(mousePress->position.x);
                    float my = static_cast<float>(mousePress->position.y);

                    // Check Help Modal Close Button click (if modal is open)
                    if (showHelpModal && mx >= 575.f && mx <= 605.f && my >= 145.f && my <= 170.f) {
                        showHelpModal = false;
                    }
                    // Check Help Button click
                    else if (mx >= 630.f && mx <= 780.f && my >= 15.f && my <= 45.f) {
                        showHelpModal = !showHelpModal;
                    } 
                    else if (!showHelpModal) {
                        // Check if clicking an existing finished shape directly on its line segments
                        bool selectedExisting = false;
                        for (size_t i = 0; i < shapes.size(); ++i) {
                            if (shapes[i].isFinished) {
                                std::vector<sf::Vector2f> shapeCurve = shapes[i].controlPoints;
                                for (int lvl = 0; lvl < shapes[i].subdivisionLevel; ++lvl) {
                                    shapeCurve = chaikinSubdivide(shapeCurve, shapes[i].isClosedLoop);
                                }

                                if (shapeCurve.size() > 1) {
                                    size_t limit = shapes[i].isClosedLoop ? shapeCurve.size() : shapeCurve.size() - 1;
                                    bool clickedOnLine = false;

                                    for (size_t j = 0; j < limit; ++j) {
                                        sf::Vector2f p0 = shapeCurve[j];
                                        sf::Vector2f p1 = shapeCurve[(j + 1) % shapeCurve.size()];
                                        
                                        float dist = pointToSegmentDistance(sf::Vector2f(mx, my), p0, p1);
                                        if (dist <= 6.0f) { // 6-pixel tolerance on line click
                                            clickedOnLine = true;
                                            break;
                                        }
                                    }

                                    if (clickedOnLine) {
                                        activeShapeIndex = static_cast<int>(i);
                                        shapes[i].isFinished = false; // Re-open for editing!
                                        selectedExisting = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if (!selectedExisting) {
                            if (activeShapeIndex == -1) {
                                // Create a new shape object
                                ShapeObject newShape;
                                newShape.controlPoints.push_back(sf::Vector2f(mx, my));
                                shapes.push_back(newShape);
                                activeShapeIndex = static_cast<int>(shapes.size() - 1);
                            } else {
                                // Check if clicking near control points of the active shape to drag
                                auto& activeShape = shapes[activeShapeIndex];
                                int foundIndex = -1;
                                for (size_t i = 0; i < activeShape.controlPoints.size(); ++i) {
                                    float dx = activeShape.controlPoints[i].x - mx;
                                    float dy = activeShape.controlPoints[i].y - my;
                                    if (std::sqrt(dx * dx + dy * dy) <= 10.f) {
                                        foundIndex = static_cast<int>(i);
                                        break;
                                    }
                                }

                                if (foundIndex != -1) {
                                    isDragging = true;
                                    draggedPointIndex = foundIndex;
                                } else {
                                    // Add point to current active shape
                                    activeShape.controlPoints.push_back(sf::Vector2f(mx, my));
                                }
                            }
                        }
                        renderCanvas();
                    }
                }
            }

            // Mouse Button Released
            if (const auto* mouseRelease = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseRelease->button == sf::Mouse::Button::Left) {
                    isDragging = false;
                    draggedPointIndex = -1;
                }
            }

            // Mouse Moved (Dragging active control point)
            if (const auto* mouseMove = event->getIf<sf::Event::MouseMoved>()) {
                if (isDragging && activeShapeIndex != -1 && draggedPointIndex >= 0) {
                    auto& activeShape = shapes[activeShapeIndex];
                    if (draggedPointIndex < static_cast<int>(activeShape.controlPoints.size())) {
                        activeShape.controlPoints[draggedPointIndex] = sf::Vector2f(static_cast<float>(mouseMove->position.x), 
                                                                                    static_cast<float>(mouseMove->position.y));
                        renderCanvas();
                    }
                }
            }

            // Keyboard Controls
            if (const auto* keyPress = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPress->code == sf::Keyboard::Key::Escape && showHelpModal) {
                    showHelpModal = false; // Allow ESC to close help too
                }
                if (keyPress->code == sf::Keyboard::Key::S) { // Subdivide Level Up
                    if (activeShapeIndex != -1) {
                        auto& activeShape = shapes[activeShapeIndex];
                        int minPoints = activeShape.isClosedLoop ? 3 : 2;
                        if (activeShape.controlPoints.size() >= minPoints && activeShape.subdivisionLevel < 5) {
                            activeShape.subdivisionLevel++;
                            renderCanvas();
                        }
                    }
                }
                if (keyPress->code == sf::Keyboard::Key::R) { // Reduce Smoothing Level Down
                    if (activeShapeIndex != -1) {
                        auto& activeShape = shapes[activeShapeIndex];
                        if (activeShape.subdivisionLevel > 0) {
                            activeShape.subdivisionLevel--;
                            renderCanvas();
                        }
                    }
                }
                if (keyPress->code == sf::Keyboard::Key::O) { // Toggle Open/Closed for active shape
                    if (activeShapeIndex != -1) {
                        shapes[activeShapeIndex].isClosedLoop = !shapes[activeShapeIndex].isClosedLoop;
                        renderCanvas();
                    }
                }
                if (keyPress->code == sf::Keyboard::Key::F) { // Finish active shape and lock it
                    if (activeShapeIndex != -1) {
                        shapes[activeShapeIndex].isFinished = true;
                        activeShapeIndex = -1;
                        isDragging = false;
                        renderCanvas();
                    }
                }
                if (keyPress->code == sf::Keyboard::Key::H) { // Toggle Original Mesh visibility
                    showOriginalMesh = !showOriginalMesh;
                    renderCanvas();
                }
                if (keyPress->code == sf::Keyboard::Key::C) { // Clear all shapes
                    shapes.clear();
                    activeShapeIndex = -1;
                    isDragging = false;
                    renderCanvas();
                }
            }
        }

        window.clear(sf::Color::White);
        window.draw(canvasSprite);

        // Draw Control Points for the currently active shape
        if (activeShapeIndex != -1 && activeShapeIndex < static_cast<int>(shapes.size())) {
            const auto& activeShape = shapes[activeShapeIndex];
            for (size_t i = 0; i < activeShape.controlPoints.size(); ++i) {
                sf::CircleShape vertexMarker(6.f);
                vertexMarker.setOrigin({6.f, 6.f});
                vertexMarker.setPosition(activeShape.controlPoints[i]);

                if (i == 0) vertexMarker.setFillColor(sf::Color::Green);
                else if (i == activeShape.controlPoints.size() - 1 && !activeShape.isClosedLoop) vertexMarker.setFillColor(sf::Color::Red);
                else vertexMarker.setFillColor(sf::Color::Magenta);

                window.draw(vertexMarker);
            }
        }

        // --- HUD DRAWING LAYER ---
        sf::Text titleText(font, "Chaikin's & Bresenham's Multi-Shape Editor", 20);
        titleText.setFillColor(sf::Color(50, 50, 50));
        titleText.setPosition({20.f, 15.f});
        window.draw(titleText);

        std::string promptStr = (activeShapeIndex != -1) ? "Editing Shape (Press 'F' when done)" : "Click to draw/start a new shape, or click directly on any shape's line to re-edit.";
        sf::Text promptText(font, promptStr, 13);
        promptText.setFillColor(sf::Color(100, 100, 100));
        promptText.setPosition({20.f, 42.f});
        window.draw(promptText);

        std::string modeStr;
        if (activeShapeIndex != -1) {
            const auto& activeShape = shapes[activeShapeIndex];
            modeStr = "Mode: " + std::string(activeShape.isClosedLoop ? "Closed" : "Open") + 
                      " | Level: " + std::to_string(activeShape.subdivisionLevel) + 
                      " | Mesh: " + std::string(showOriginalMesh ? "Visible" : "Hidden");
        } else {
            modeStr = "Status: Ready (Mesh: " + std::string(showOriginalMesh ? "Visible" : "Hidden") + ")";
        }
        sf::Text modeText(font, modeStr, 13);
        modeText.setFillColor(sf::Color(0, 120, 200));
        modeText.setPosition({20.f, 62.f});
        window.draw(modeText);

        // Help Button Box [?]
        sf::RectangleShape helpButton(sf::Vector2f(155.f, 30.f));
        helpButton.setPosition({630.f, 15.f});
        helpButton.setFillColor(sf::Color(70, 130, 180));
        helpButton.setOutlineColor(sf::Color(40, 90, 140));
        helpButton.setOutlineThickness(1.f);
        window.draw(helpButton);

        sf::Text helpButtonText(font, "Click for instructions", 14);
        helpButtonText.setFillColor(sf::Color::White);
        helpButtonText.setPosition({640.f, 21.f});
        window.draw(helpButtonText);

        // Instruction Popup Modal
        if (showHelpModal) {
            sf::RectangleShape modalBg(sf::Vector2f(440.f, 290.f));
            modalBg.setPosition({180.f, 140.f});
            modalBg.setFillColor(sf::Color(245, 245, 245, 245));
            modalBg.setOutlineColor(sf::Color(100, 100, 100));
            modalBg.setOutlineThickness(2.f);
            window.draw(modalBg);

            std::string instructions = 
                "Instructions\n\n"
                "Left-Click: Add points / Click line to select shape\n"
                "Drag Points: Reshape active drawing\n"
                "'S' Key: Smooth the shape\n"
                "'R' Key: Reduce Smoothing level\n"
                "'O' Key: Toggle Open Path / Closed Loop\n"
                "'F' Key: Finish current shape (Lock & deselect)\n"
                "'H' Key: Hide / Show original control polygon\n"
                "'C' Key: Clear all shapes";

            sf::Text modalText(font, instructions, 13);
            modalText.setFillColor(sf::Color::Black);
            modalText.setPosition({200.f, 160.f});
            window.draw(modalText);

            // Modal Close Button 
            sf::RectangleShape closeBtn(sf::Vector2f(30.f, 25.f));
            closeBtn.setPosition({575.f, 145.f});
            closeBtn.setFillColor(sf::Color(200, 60, 60));
            window.draw(closeBtn);

            sf::Text closeBtnText(font, "X", 14);
            closeBtnText.setFillColor(sf::Color::White);
            closeBtnText.setPosition({585.f, 148.f});
            window.draw(closeBtnText);
        }

        window.display();
    }

    return 0;
}