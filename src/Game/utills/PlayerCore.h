#pragma once
#include <SFML/Graphics.hpp>



struct PlayerCore {
    sf::CircleShape hitbox;
    sf::Vector2f velocity;
    float speed = 5.0f;
    int health = 100;
};