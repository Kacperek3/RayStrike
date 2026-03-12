#pragma once
#include <SFML/Graphics.hpp>

struct PlayerRender {
    sf::Sprite *bodySprite;
    sf::Sprite *gunSprite;
    sf::Text *indicatorText;
    sf::RectangleShape healthBarBackground;
    sf::RectangleShape healthBarFill;
};