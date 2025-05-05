#pragma once

#include <SFML/Graphics.hpp>
class InputManager {
public:
    InputManager() {}
    ~InputManager() {}

    bool IsSpriteClicked(sf::Sprite object, sf::Mouse::Button button, sf::RenderWindow &window);
    bool IsSpriteClickedAccurate(sf::Sprite object, sf::Mouse::Button button, sf::RenderWindow &window);
    bool IsSpriteHover(sf::Sprite object, sf::RenderWindow &window);
    bool IsSpriteHoverAccurate(sf::Sprite object,
                         sf::RenderWindow &window,
                         float detectionRadius = 0.0f);
    bool IsTextClicked(sf::Text text, sf::Mouse::Button button, sf::RenderWindow &window);
    sf::Vector2f GetMousePosition(sf::RenderWindow &window);
};