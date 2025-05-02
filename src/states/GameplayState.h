#pragma once
#include <SFML/Graphics.hpp>
#include "Game.h"
#include "State.h"
#include <unordered_map>
#include <ctime>
#include <cmath>

#include "NetworkManager.h"
#include "NetworkGameManager.h"


class GameplayState : public State {
public:
    GameplayState(GameDataRef data);
    ~GameplayState();
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

    sf::Font _font;
    sf::Text *_titleText;

    const float _exitAnimationSpeed = 80.0f;
    sf::CircleShape *_playerCircle;
    sf::CircleShape *_enemyCircle;
    sf::Clock *_clock;

    NetworkGameManager* _networkManager;


};
