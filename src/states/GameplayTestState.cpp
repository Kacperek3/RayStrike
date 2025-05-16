#include "GameplayTestState.h"

GameplayTestState::GameplayTestState(GameDataRef data, int serverTcpSocket, int clientTcpSocket) : _data(data), clientTcpSocket(clientTcpSocket), serverTcpSocket(serverTcpSocket) {
    srand(static_cast<unsigned>(time(NULL)));
    _titleText = new sf::Text();
    _playerCircle = new sf::CircleShape();
    _enemyCircle = new sf::CircleShape();
    _udpNetworkManager = new UdpNetworkManager(serverTcpSocket, clientTcpSocket);

    _clock = new sf::Clock();
}

void GameplayTestState::Init(){
    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }
    

    _data->assetManager.LoadTexture("background", "assets/background.jpg");
    _data->assetManager.LoadTexture("reloadLobbiesButton", "assets/flair_arrow_3.png");
    _data->assetManager.LoadTexture("reloadLobbiesButtonHover", "assets/flair_arrow_3_hover.png");

    _titleText->setFont(_font);
    _titleText->setString("Game");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(sf::Color::White);


    _titleText->setPosition(_data->window.getSize().x,60);

    _playerCircle->setRadius(20);
    _playerCircle->setFillColor(sf::Color(255, 0, 0));
    _playerCircle->setPosition(100, 100);
    _playerCircle->setOrigin(_playerCircle->getRadius(), _playerCircle->getRadius());
    _playerCircle->setOutlineThickness(2);
    _playerCircle->setOutlineColor(sf::Color(0, 0, 0));
    _playerCircle->setPointCount(30);

    _enemyCircle->setRadius(20);
    _enemyCircle->setFillColor(sf::Color(0, 0, 255));
    _enemyCircle->setPosition(200, 200);
    _enemyCircle->setOrigin(_enemyCircle->getRadius(), _enemyCircle->getRadius());
    _enemyCircle->setOutlineThickness(2);
    _enemyCircle->setOutlineColor(sf::Color(0, 0, 0));
    _enemyCircle->setPointCount(30);


    if (!_udpNetworkManager->Initialize()) {
        std::cerr << "Blad inicjalizacji UDP\n";
        close(clientTcpSocket);
        close(serverTcpSocket);
    }
    std::cout << "Polaczenie UDP nawiazane!\n";
}

void GameplayTestState::HandleInput() {
    sf::Event event;
    while (_data->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _data->window.close();
            return;
        }
        if(event.type == sf::Event::MouseButtonPressed ) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
                }
            }
            return;
        }
        if(event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                _animationState = AnimationState::EXITING;
            }
        }
}

void GameplayTestState::Update() {
    const float moveSpeed = 200.0f; // piksele na sekundę

    float deltaTime = _clock->restart().asSeconds(); // czas od ostatniej klatki

    sf::Vector2f movement(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        movement.y -= moveSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        movement.y += moveSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        movement.x -= moveSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        movement.x += moveSpeed * deltaTime;
    }

    _playerCircle->move(movement);

    // Animation logic
    if (_animationState == AnimationState::ENTERING) {
        enteringAnimation();
    } else if (_animationState == AnimationState::EXITING) {
        exitingAnimation();
    } else {
        standartAnimation();
    }


   std::string received;
    if (_udpNetworkManager->WaitForMessage(received, 100)) {
        // Przetwarzanie danych (np. pozycja gracza)
        size_t sep = received.find(',');
        if (sep != std::string::npos) {
            float x = std::stof(received.substr(0, sep));
            float y = std::stof(received.substr(sep+1));
            
            _enemyCircle->setPosition(x, y);
        }
    }
    _udpNetworkManager->Send(std::to_string(_playerCircle->getPosition().x) + "," + std::to_string(_playerCircle->getPosition().y));

}

void GameplayTestState::exitingAnimation() {
    
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

void GameplayTestState::enteringAnimation() {
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

void GameplayTestState::standartAnimation(){
    sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);

    return;
}

void GameplayTestState::Draw() {
    _data->window.clear();
    _data->window.draw(*_titleText);
    _data->window.draw(*_playerCircle);
    _data->window.draw(*_enemyCircle);


    _data->window.display();
}


GameplayTestState::~GameplayTestState() {
    delete _titleText;
    delete _playerCircle;
    delete _enemyCircle;
    delete _clock;
    delete _udpNetworkManager;
}


void GameplayTestState::ClearObjects() {

}