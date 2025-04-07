#pragma once
#include <SFML/Graphics.hpp>
#include "Game.h"
#include "State.h"
#include <unordered_map>
#include <ctime>
#include <cmath>


class MenuState : public State {
public:
    MenuState(GameDataRef data);
    ~MenuState();
    void Init() override;

    void HandleInput() override;
    void Update() override;
    void Draw() override;
    void ClearObjects() override;


private:
    struct Bullet {
        sf::Sprite sprite;
        sf::Vector2f velocity;
        bool isGroupBullet = false;
    };

    void ResetBullet(Bullet& bullet, bool initialSpawn = false);


    GameDataRef _data;
    sf::Font _font;
    sf::Sprite *_backgroundTexture;

    sf::Text *_titleText;
    sf::Text *_copyrightText;

    sf::RectangleShape *_createGameButton;
    sf::Text *_createGameButtonText;

    sf::RectangleShape *_joinGameButton;
    sf::Text *_joinGameButtonText;

    sf::RectangleShape *_settingsButton;
    sf::Text *_settingsButtonText;

    sf::RectangleShape *_exitButton;
    sf::Text *_exitButtonText;
    
    std::unordered_map<sf::RectangleShape*, std::tuple<sf::Vector2f, sf::Vector2f, sf::FloatRect, sf::Color>> _buttonData;

    bool _isExiting = false;
    bool isComingFromSettings = false;
    const float _exitAnimationSpeed = 50.0f;

    

    std::vector<Bullet> _bullets;
    float _bulletSpeed;
    sf::Vector2u _windowSize;
    float _groupSpawnChance = 0.3f;
    int _maxGroupSize = 5;


};
