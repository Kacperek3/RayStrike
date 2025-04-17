#pragma once
#include <SFML/Graphics.hpp>

#include <unordered_map>
#include <ctime>
#include <cmath>

#include "Game.h"
#include "State.h"
#include "TextField.h"
#include "LabeledTextField.h"




class CreateLobbyState : public State {
public:
    CreateLobbyState(GameDataRef data);
    ~CreateLobbyState();
    void Init() override;

    void HandleInput() override;
    void Update() override;
    void Draw() override;
    void ClearObjects() override;


private:

    enum class AnimationState {
        ENTERING,
        EXITING,
        NONE
    };


    void exitingAnimation();
    void enteringAnimation();
    void standartAnimation();


    GameDataRef _data;
    AnimationState _animationState = AnimationState::ENTERING;
    std::unordered_map<sf::RectangleShape*, std::tuple<sf::Vector2f, sf::Vector2f, sf::FloatRect, sf::Color>> _buttonData;


    sf::Font _font;
    sf::Sprite *_backgroundTexture;
    sf::Text *_titleText;

    sf::RectangleShape *_backButton;
    sf::Text *_backButtonText;

    
    LabeledTextField *_lobbyNameTextField;
    LabeledTextField *_lobbyPlayerNameTextField;

    const float _exitAnimationSpeed = 80.0f;
};
