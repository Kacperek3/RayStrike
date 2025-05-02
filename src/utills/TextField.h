#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class TextField {
public:
    TextField(float x, float y, float width, float height, sf::Font& font, unsigned int charSize = 24);
    void handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window);
    std::string getInput() const;
    void setPosition(float x, float y);
    void setPosition(sf::Vector2f pos) { setPosition(pos.x, pos.y); }
    sf::FloatRect getGlobalBounds() const;
    sf::Vector2f getPosition() const;

private:
    sf::RectangleShape box;
    sf::Text text;
    std::string input;
    bool isActive;
};
