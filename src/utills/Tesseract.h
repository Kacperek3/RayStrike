#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <array>

struct Vector4D {
    float x, y, z, w;
};

using Edge = std::pair<size_t, size_t>;

class Tesseract {
public:
    Tesseract();
    void update();
    void draw(sf::RenderWindow& window);
    void setPosition(const sf::Vector2f& position);
    void setScale(float scale);

private:
    std::vector<Vector4D> vertices;
    std::vector<Edge> edges;
    float angleXW;
    float angleYW;
    sf::Vector2f position;
    float scale;
    
    sf::Vector2f project4DTo2D(const Vector4D& v);
};