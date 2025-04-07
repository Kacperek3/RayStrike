#pragma once
#include <SFML/Graphics.hpp>
#include "Game.h"
#include "State.h"
#include <unordered_map>
#include <ctime>
#include <cmath>


class SettingsState : public State {
public:
    SettingsState(GameDataRef data);
    ~SettingsState();
    void Init() override;

    void HandleInput() override;
    void Update() override;
    void Draw() override;
    void ClearObjects() override;


private:

    GameDataRef _data;
    sf::Font _font;
    sf::Sprite *_backgroundTexture;
    sf::Text *_titleText;

    const float _exitAnimationSpeed = 80.0f;
    sf::Vector2f _titleTargetPos;
};
