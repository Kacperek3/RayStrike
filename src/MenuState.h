#pragma once
#include <SFML/Graphics.hpp>
#include "Game.h"
#include "State.h"
#include <unordered_map>


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
    GameDataRef _data;
    sf::Font _font;
    sf::Text *_titleText;

    sf::RectangleShape *_createGameButton;
    sf::Text *_createGameButtonText;

    sf::RectangleShape *_joinGameButton;
    sf::Text *_joinGameButtonText;

    sf::RectangleShape *_settingsButton;
    sf::Text *_settingsButtonText;

    sf::RectangleShape *_exitButton;
    sf::Text *_exitButtonText;
    
    std::unordered_map<sf::RectangleShape*, std::tuple<sf::Vector2f, sf::Vector2f, sf::FloatRect, sf::Color>> _buttonData;

    sf::Sound *_soundtrack;
};
