#include "LobbyWaitingState.h"

LobbyWaitingState::LobbyWaitingState(GameDataRef data, std::string lobbyName, std::string playerName)
    : _data(data), _lobbyName(lobbyName), _playerName(playerName) {
    srand(static_cast<unsigned>(time(NULL)));

    _backgroundTexture = new sf::Sprite();
    _titleText = new sf::Text();
}

void LobbyWaitingState::Init() {
    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }

    _data->assetManager.LoadTexture("background", "assets/background.jpg");
    _backgroundTexture->setTexture(_data->assetManager.GetTexture("background"));

    _titleText->setFont(_font);
    _titleText->setString("Waiting for players...");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(sf::Color::White);
    _titleText->setPosition(500, 100);

    _waitingText.setFont(_font);
    _waitingText.setString("Waiting for players...");
    _waitingText.setCharacterSize(30);
    _waitingText.setFillColor(sf::Color::White);
    _waitingText.setPosition(500, 300);


    _udpSocket.setBlocking(false);
    _udpSocket.bind(sf::Socket::AnyPort);

    if (_listener.listen(_listenPort) != sf::Socket::Done) {
        std::cout << "TCP Listener failed!" << std::endl;
    }
    _listener.setBlocking(false); 

    _broadcastClock = new sf::Clock();
    _broadcastClock->restart();
}

void LobbyWaitingState::HandleInput() {
    sf::Event event;
    while (_data->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _data->window.close();
            return;
        }
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
                _animationState = AnimationState::EXITING;
            }
            return;
        }
    }
}

void LobbyWaitingState::Update() {
    // if (_animationState == AnimationState::ENTERING) {
    //     enteringAnimation();
    // } else if (_animationState == AnimationState::EXITING) {
    //     exitingAnimation();
    // } else {
    //     standartAnimation();
    // }

    // CO 1 SEKUNDĘ WYŚLIJ BROADCAST
    if (_broadcastClock->getElapsedTime().asSeconds() >= 1.0f) {
        std::string message = "LOBBY|" + _lobbyName + "|" + _playerName;
        if (_udpSocket.send(message.c_str(), message.size(), sf::IpAddress::Broadcast, _broadcastPort) != sf::Socket::Done) {
            std::cout << "Failed to send UDP broadcast\n";
        }
        _broadcastClock->restart();
    }

    // SPRAWDŹ, CZY KLIENT SIĘ POŁĄCZYŁ
    if (!_connected) {
        if (_listener.accept(_client) == sf::Socket::Done) {
            _connected = true;
            std::cout << "Gracz dołączył z IP: " << _client.getRemoteAddress() << std::endl;
            _client.setBlocking(false);

          
        }
    }
}


void LobbyWaitingState::Draw() {
    _data->window.clear();
    _data->window.draw(*_backgroundTexture);
    _data->window.draw(*_titleText);
    _data->window.draw(_waitingText);
    _data->window.display();
}


LobbyWaitingState::~LobbyWaitingState() {
    delete _backgroundTexture;
    delete _titleText;
    delete _broadcastClock;
}

