#pragma once
#include <SFML/Graphics.hpp>
#include "Game.h"
#include "State.h"
#include <unordered_map>
#include <ctime>
#include <cmath>

#include "UdpNetworkManager.h"


class GameplayTestState : public State {
public:
    GameplayTestState(GameDataRef data, int serverTcpSocket, int clientTcpSocket);
    ~GameplayTestState();
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

    UdpNetworkManager* _udpNetworkManager;
    int serverTcpSocket;
    int clientTcpSocket;



};
