#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "Game.h"
#include "State.h"
#include <unordered_map>
#include <ctime>
#include <cmath>



class JoinLobbyState : public State {
public:
    JoinLobbyState(GameDataRef data);
    ~JoinLobbyState();
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
    sf::Sprite *_backgroundTexture;
    sf::Text *_titleText;

    const float _exitAnimationSpeed = 80.0f;


    sf::UdpSocket _udpSocket;
    sf::Clock *_receiveClock;

    struct LobbyInfo {
        std::string name;
        std::string hostPlayer;
        sf::IpAddress ip;
        sf::Time lastSeen;
    };

    std::unordered_map<std::string, LobbyInfo> _lobbies;

    const unsigned short _broadcastPort = 54001;
    const unsigned short _connectPort = 54000;


};
