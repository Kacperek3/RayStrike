#include "JoinLobbyState.h"

JoinLobbyState::JoinLobbyState(GameDataRef data) : _data(data) {
    srand(static_cast<unsigned>(time(NULL)));

    _backgroundTexture = new sf::Sprite();
    _titleText = new sf::Text();
}

void JoinLobbyState::Init(){
    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }

    _data->assetManager.LoadTexture("background", "assets/background.jpg");
    _backgroundTexture->setTexture(_data->assetManager.GetTexture("background"));

    _titleText->setFont(_font);
    _titleText->setString("Join Lobby");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(sf::Color::White);


    _titleText->setPosition(_data->window.getSize().x,100);


    _udpSocket.setBlocking(false);
    if (_udpSocket.bind(_broadcastPort) != sf::Socket::Done) {
        std::cout << "Failed to bind UDP socket!" << std::endl;
    }

    _receiveClock = new sf::Clock();
    _receiveClock->restart();
   
}

void JoinLobbyState::HandleInput() {
    sf::Event event;
    while (_data->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _data->window.close();
            return;
        }
        if(event.type == sf::Event::MouseButtonPressed ) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
                _animationState = AnimationState::EXITING;
            }
            return;
        }
    }
}

void JoinLobbyState::Update() {

    // Animation logic
    if(_animationState == AnimationState::ENTERING) {
        enteringAnimation();
    } else if(_animationState == AnimationState::EXITING) {
        exitingAnimation();
    } else {
        standartAnimation();
    }



    char buffer[128];
    std::size_t received = 0;
    sf::IpAddress sender;
    unsigned short port;

    if (_udpSocket.receive(buffer, sizeof(buffer), received, sender, port) == sf::Socket::Done) {
        std::string msg(buffer, received);
        if (msg.find("LOBBY|") == 0) {
            auto parts = msg.substr(6); // usuń "LOBBY|"
            auto delimPos = parts.find('|');
            if (delimPos != std::string::npos) {
                std::string lobbyName = parts.substr(0, delimPos);
                std::string hostPlayer = parts.substr(delimPos + 1);

                _lobbies[lobbyName] = LobbyInfo{lobbyName, hostPlayer, sender, _receiveClock->getElapsedTime()};
            }
        }
    }

    // Czyść nieaktywne lobby po 5 sek
    for (auto it = _lobbies.begin(); it != _lobbies.end();) {
        if (_receiveClock->getElapsedTime() - it->second.lastSeen > sf::seconds(5)) {
            it = _lobbies.erase(it);
        } else {
            ++it;
        }
    }


    int i = 0;
    for (const auto& [key, lobby] : _lobbies) {
        std::string text = "Lobby: " + lobby.name + " | Host: " + lobby.hostPlayer;
        std::cout << text << std::endl;
    }
    
}


void JoinLobbyState::exitingAnimation() {
    
    sf::Vector2f currentPos = _titleText->getPosition();

    if (currentPos.x < _data->window.getSize().x) {
        currentPos.x += _exitAnimationSpeed;
        if (currentPos.x >= _data->window.getSize().x){
            currentPos.x = _data->window.getSize().x;
            _animationState = AnimationState::ENTERING;
            _data->stateManager.RemoveState();
        }
        
        _titleText->setPosition(currentPos);

    }
    
    return;
}

void JoinLobbyState::enteringAnimation() {
    sf::Vector2f currentPos = _titleText->getPosition();

    if (currentPos.x > 0) {
        currentPos.x -= _exitAnimationSpeed;
        if (currentPos.x <= 500){
            currentPos.x = 500;
            _animationState = AnimationState::NONE;
        }
        
        _titleText->setPosition(currentPos);

    }
}

void JoinLobbyState::standartAnimation(){
    return;
}




void JoinLobbyState::Draw() {
    _data->window.clear();
    _data->window.draw(*_backgroundTexture);
    _data->window.draw(*_titleText);

    _data->window.display();
}


JoinLobbyState::~JoinLobbyState() {
    delete _backgroundTexture;
    delete _titleText;
    delete _receiveClock;
    
}


void JoinLobbyState::ClearObjects() {
   // _data->assetManager.clearAssets();
    //_data->soundManager.ClearSounds();
}