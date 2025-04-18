#include "LobbyWaitingState.h"

LobbyWaitingState::LobbyWaitingState(GameDataRef data, std::string lobbyName, std::string playerName)
    : _data(data), _lobbyName(lobbyName), _playerName(playerName) {
    
    _backButton = new sf::RectangleShape();
    _backButtonText = new sf::Text();
    _centralWaitingText = new sf::Text();

    _backgroundTexture = new sf::Sprite();
    _titleText = new sf::Text();
    _tesseract = new Tesseract();

    _dotClock = new sf::Clock();
}

void LobbyWaitingState::Init() {
    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }

    _data->assetManager.LoadTexture("background", "assets/background.jpg");
    _backgroundTexture->setTexture(_data->assetManager.GetTexture("background"));

    _titleText->setFont(_font);
    _titleText->setString("Lobby Created");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(sf::Color::White);
    _titleText->setPosition(_data->window.getSize().x, 100);
    
    _centralWaitingText->setFont(_font);
    _centralWaitingText->setString("Waiting for players");
    _centralWaitingText->setCharacterSize(25);
    _centralWaitingText->setFillColor(sf::Color::Red);
    _centralWaitingText->setPosition(_data->window.getSize().x, 400);


    _tesseract->setPosition(sf::Vector2f(620, 400));
    _tesseract->setScale(3.0f);

    //buttons
    _backButton->setSize(sf::Vector2f(250, 50));
    _backButton->setFillColor(sf::Color(80, 150, 255,150));
    _backButton->setPosition(sf::Vector2f(515, 700));

    _backButtonText->setFont(_font);
    _backButtonText->setString("Back");
    _backButtonText->setCharacterSize(30);
    _backButtonText->setFillColor(sf::Color::White);
    _backButtonText->setPosition(_backButton->getPosition().x + 70, _backButton->getPosition().y + 8);

    auto storeButtonData = [&](sf::RectangleShape* btn, sf::Text* txt) {
        sf::Vector2f originalBtnPos = btn->getPosition();
        sf::Vector2f originalTxtPos = txt->getPosition();
        sf::FloatRect originalBounds = btn->getGlobalBounds();
        sf::Color originalColor = btn->getFillColor();
        _buttonData[btn] = {originalBtnPos, originalTxtPos, originalBounds, originalColor};
    };
    storeButtonData(_backButton, _backButtonText);  

    _backButton->setPosition(1100, 700);
    _backButtonText->setPosition(1170, 708);
    



    // network
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    int broadcastEnable = 1;
    setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(8888);
    broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");

    // Inicjalizacja TCP
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&tcpAddr, 0, sizeof(tcpAddr));
    tcpAddr.sin_family = AF_INET;
    tcpAddr.sin_addr.s_addr = INADDR_ANY;
    tcpAddr.sin_port = htons(0); // Losowy port

    if (bind(tcpSocket, (sockaddr*)&tcpAddr, sizeof(tcpAddr)) < 0) {
        std::cerr << "Błąd bindowania TCP\n";
        return;
    }

    // Pobierz przypisany port
    socklen_t len = sizeof(tcpAddr);
    getsockname(tcpSocket, (sockaddr*)&tcpAddr, &len);
    tcpPort = ntohs(tcpAddr.sin_port);

    listen(tcpSocket, SOMAXCONN);

    // Przygotuj wiadomość broadcast
    broadcastMessage = _lobbyName +"|"+ _playerName + "|" + std::to_string(tcpPort);
    broadcastClock.restart();
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
                if (_backButton->getGlobalBounds().contains(mousePos)) {
                    _animationState = AnimationState::EXITING;
                    _data->stateManager.RemoveState();
                }
            }
            return;
        }
    }
}

void LobbyWaitingState::Update() {
    if (_animationState == AnimationState::ENTERING) {
        enteringAnimation();
    } else if (_animationState == AnimationState::EXITING) {
        exitingAnimation();
    } else {
        standartAnimation();
    }
    _tesseract->update();




    if (broadcastClock.getElapsedTime().asSeconds() >= 1.0f) {
        sendto(udpSocket, broadcastMessage.c_str(), broadcastMessage.size(), 0,
               (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
        broadcastClock.restart();
    }

    // Sprawdź połączenia TCP
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(tcpSocket, &readSet);
    timeval timeout = {0, 0};

    if (select(tcpSocket + 1, &readSet, nullptr, nullptr, &timeout) > 0) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int client = accept(tcpSocket, (sockaddr*)&clientAddr, &clientLen);

        if (client >= 0) {
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);
            std::cout << "Gracz dołączył: " << ip << std::endl;
            close(client);

            
        }
    }
}



void LobbyWaitingState::exitingAnimation() {
    
    
    
    return;
}

void LobbyWaitingState::enteringAnimation() {
    bool allButtonsOffScreen = true;
    bool allTextFieldsOffScreen = true;

    for (auto& [button, data] : _buttonData) {
        auto& [originalBtnPos, originalTxtPos, originalBounds, originalColor] = data;
        
        sf::Vector2f newPos = button->getPosition();
        newPos.x -= _exitAnimationSpeed;
        button->setPosition(newPos);
        _titleText->setPosition(_titleText->getPosition().x - _exitAnimationSpeed, _titleText->getPosition().y);


        sf::Text* text = nullptr;
        if (button == _backButton) text = _backButtonText;
        
        if (text) {
            sf::Vector2f textPos = text->getPosition();
            textPos.x -= _exitAnimationSpeed;
            text->setPosition(textPos);
        }

        sf::FloatRect bounds = button->getGlobalBounds();
        if (bounds.left + bounds.width > 0) {
            allButtonsOffScreen = false;
        }
    }


    sf::Vector2f newPos = _centralWaitingText->getPosition();
    
    if(newPos.x > 0){
        newPos.x -= _exitAnimationSpeed + 50;
        if(newPos.x <= 495){
            newPos.x = 495;
            allTextFieldsOffScreen = false;
        }

        if(newPos.x >= 495) _centralWaitingText->setPosition(newPos);
    }


    sf::Vector2f currentPos = _titleText->getPosition();

    if (currentPos.x > 0) {
        currentPos.x -= _exitAnimationSpeed;
        if (currentPos.x <= 405 && !allButtonsOffScreen && !allTextFieldsOffScreen) {
            currentPos.x = 405;
            _animationState = AnimationState::NONE;
        }
        
        _titleText->setPosition(currentPos);

    }
}

void LobbyWaitingState::standartAnimation(){
    if (_dotClock->getElapsedTime().asSeconds() >= 0.1f) {
        _dotClock->restart();
        
        _charIndex++;
        if (_charIndex > _baseText.size()) {
            _charIndex = 0;
            _centralWaitingText->setString("");
        } else {
            _centralWaitingText->setString(_baseText.substr(0, _charIndex));
        }
    }


    sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
    const float hoverOffset = 10.f;
    const sf::Color hoverColor(0, 59, 190);
    for (auto& [button, data] : _buttonData) {
        auto& [originalBtnPos, originalTxtPos, originalBounds, originalColor] = data;
        sf::Text* text = nullptr;

        if (button == _backButton) text = _backButtonText;
        
        if (originalBounds.contains(mousePos)) {
            button->setPosition(originalBtnPos.x, originalBtnPos.y);
            button->setFillColor(hoverColor);
            if (text) text->setPosition(originalTxtPos.x, originalTxtPos.y);
        } else {
            button->setPosition(originalBtnPos);
            button->setFillColor(originalColor);
            if (text) text->setPosition(originalTxtPos);
        }
    }
}


void LobbyWaitingState::Draw() {
    _data->window.clear();
    _data->window.draw(*_backgroundTexture);
    _tesseract->draw(_data->window);
    _data->window.draw(*_titleText);
    _data->window.draw(*_centralWaitingText);
    _data->window.draw(*_backButton);
    _data->window.draw(*_backButtonText);
    _data->window.display();
}


LobbyWaitingState::~LobbyWaitingState() {
    delete _backgroundTexture;
    delete _tesseract;
    delete _titleText;
    delete _centralWaitingText;
    delete _backButton;
    delete _backButtonText;
    delete _dotClock;

    close(udpSocket);
    close(tcpSocket);
}

