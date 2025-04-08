#include "CreateLobbyState.h"

CreateLobbyState::CreateLobbyState(GameDataRef data) : _data(data) {
    srand(static_cast<unsigned>(time(NULL)));

    _backgroundTexture = new sf::Sprite();
    _titleText = new sf::Text();
}

void CreateLobbyState::Init(){
    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }

    _data->assetManager.LoadTexture("background", "assets/background.jpg");
    _backgroundTexture->setTexture(_data->assetManager.GetTexture("background"));

    _titleText->setFont(_font);
    _titleText->setString("Create Lobby");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(sf::Color::White);


    _titleText->setPosition(_data->window.getSize().x,100);
   
}

void CreateLobbyState::HandleInput() {
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

void CreateLobbyState::Update() {

    // Animation logic
    if(_animationState == AnimationState::ENTERING) {
        enteringAnimation();
    } else if(_animationState == AnimationState::EXITING) {
        exitingAnimation();
    } else {
        standartAnimation();
    }

    
}


void CreateLobbyState::exitingAnimation() {
    
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

void CreateLobbyState::enteringAnimation() {
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

void CreateLobbyState::standartAnimation(){
    return;
}




void CreateLobbyState::Draw() {
    _data->window.clear();
    _data->window.draw(*_backgroundTexture);
    _data->window.draw(*_titleText);

    _data->window.display();
}


CreateLobbyState::~CreateLobbyState() {
    delete _backgroundTexture;
    delete _titleText;
    
}


void CreateLobbyState::ClearObjects() {
   // _data->assetManager.clearAssets();
    //_data->soundManager.ClearSounds();
}