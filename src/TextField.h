#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class TextField {
public:
    TextField(float x, float y, float width, float height, sf::Font& font, unsigned int charSize = 24);
    void handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window);
    std::string getInput() const;

private:
    sf::RectangleShape box;
    sf::Text text;
    std::string input;
    bool isActive;
};
