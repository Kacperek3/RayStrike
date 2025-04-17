#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Window.hpp>

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

    GameDataRef _data;
    std::string _lobbyName;
    std::string _playerName;
    sf::Text _waitingText;

    sf::Font _font;
    sf::Sprite *_backgroundTexture;
    sf::Text *_titleText;

    const float _exitAnimationSpeed = 80.0f;

    // Networking (host)
    sf::TcpListener _listener;
    sf::TcpSocket _client;

    sf::UdpSocket _udpSocket;
    sf::Clock *_broadcastClock;

    unsigned short _broadcastPort = 54001;
    unsigned short _listenPort = 54000;


    bool _connected = false;
};
