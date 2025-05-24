#include "Tesseract.h"

Tesseract::Tesseract() : angleXW(0.0f), angleYW(0.0f), position(0, 0), scale(1.0f) {
    for(int i = 0; i < 16; ++i) {
        vertices.push_back({
            (i & 1) ? 1.0f : -1.0f,
            (i & 2) ? 1.0f : -1.0f,
            (i & 4) ? 1.0f : -1.0f,
            (i & 8) ? 1.0f : -1.0f
        });
    }
    
    for(int d = 0; d < 4; ++d) {
        int bit = 1 << d;
        for(int i = 0; i < 16; ++i) {
            int j = i ^ bit;
            if(i < j) edges.emplace_back(i, j);
        }
    }
}

void Tesseract::update() {
    angleXW += 0.005f;
    angleYW += 0.005f;
}

void Tesseract::draw(sf::RenderWindow& window) {
    std::vector<sf::Vector2f> projected;
    for(auto& v : vertices) {
        projected.push_back(project4DTo2D(v));
    }

    //sf::Color lineColor = sf::Color(255, 255, 255, 40);
    for(auto& edge : edges) {
        sf::Vertex line[] = {
            sf::Vertex(projected[edge.first], this->color),
            sf::Vertex(projected[edge.second],this->color)
        };
        window.draw(line, 2, sf::Lines);
    }
}

sf::Vector2f Tesseract::project4DTo2D(const Vector4D& v) {
    Vector4D rotated = v;
    
    float x = rotated.x * cos(angleXW) - rotated.w * sin(angleXW);
    float w = rotated.x * sin(angleXW) + rotated.w * cos(angleXW);
    rotated.x = x;
    rotated.w = w;
    
    float y = rotated.y * cos(angleYW) - rotated.w * sin(angleYW);
    w = rotated.y * sin(angleYW) + rotated.w * cos(angleYW);
    rotated.y = y;
    rotated.w = w;

    const float projection_scale = 1.0f / (rotated.w + 3.0f);
    float x3D = rotated.x * projection_scale;
    float y3D = rotated.y * projection_scale;
    float z3D = rotated.z * projection_scale;

    const float fov = 500.0f;
    const float camera_z = 3.0f;
    float depth = z3D + camera_z;
    if(depth <= 0) depth = 0.001f;
    
    sf::Vector2f result(
        x3D * fov / depth,
        y3D * fov / depth
    );
    
    return sf::Vector2f(
        result.x * scale + position.x,
        result.y * scale + position.y
    );
}

void Tesseract::setPosition(const sf::Vector2f& position) {
    this->position = position;
}

void Tesseract::setScale(float scale) {
    this->scale = scale;
}
void Tesseract::setColor(const sf::Color& color) {
    this->color = color;
}
