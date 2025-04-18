#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <unordered_map>
#include <ctime>
#include <cmath>

#include "Game.h"
#include "State.h"
#include "TextField.h"
#include "LabeledTextField.h"
#include "Tesseract.h"



class LobbyWaitingState : public State {
public:
    LobbyWaitingState(GameDataRef data, std::string lobbyName, std::string playerName);
    ~LobbyWaitingState();

    void Init() override;
    void HandleInput() override;
    void Update() override;
    void Draw() override;

private:

    enum class AnimationState {
        ENTERING,
        EXITING,
        NONE
    };


    void exitingAnimation();
    void enteringAnimation();
    void standartAnimation();

    AnimationState _animationState = AnimationState::ENTERING;
    std::unordered_map<sf::RectangleShape*, std::tuple<sf::Vector2f, sf::Vector2f, sf::FloatRect, sf::Color>> _buttonData;

    GameDataRef _data;
    std::string _lobbyName;
    std::string _playerName;

    sf::Font _font;
    sf::Sprite *_backgroundTexture;
    sf::Text *_titleText;

    const float _exitAnimationSpeed = 60.0f;

    sf::RectangleShape *_backButton;
    sf::Text *_backButtonText;

};
