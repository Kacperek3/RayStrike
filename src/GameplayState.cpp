#include "GameplayState.h"

GameplayState::GameplayState(GameDataRef data) : _data(data) {
    srand(static_cast<unsigned>(time(NULL)));
    _titleText = new sf::Text();
    _playerCircle = new sf::CircleShape();
    _enemyCircle = new sf::CircleShape();
    _networkManager = new NetworkGameManager(true, "192.168.107.161", 54000);

    _clock = new sf::Clock();
}

void GameplayState::Init(){
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


}

void GameplayState::HandleInput() {
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

void GameplayState::Update() {
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


   float otherX, otherY;
    _networkManager->GetEnemyPosition(otherX, otherY);
    _enemyCircle->setPosition(otherX, otherY);

    // Wyślij pozycję gracza
    _networkManager->SendPosition(
        _playerCircle->getPosition().x,
        _playerCircle->getPosition().y
    );
    

}

void GameplayState::exitingAnimation() {
    
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

void GameplayState::enteringAnimation() {
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

void GameplayState::standartAnimation(){
    sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);

    return;
}

void GameplayState::Draw() {
    _data->window.clear();
    _data->window.draw(*_titleText);
    _data->window.draw(*_playerCircle);
    _data->window.draw(*_enemyCircle);


    _data->window.display();
}


GameplayState::~GameplayState() {
    delete _titleText;
    delete _playerCircle;
    delete _enemyCircle;
    delete _clock;
    delete _networkManager;
    
}


void GameplayState::ClearObjects() {

}