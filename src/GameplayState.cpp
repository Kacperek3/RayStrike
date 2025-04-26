#include "GameplayState.h"

GameplayState::GameplayState(GameDataRef data) : _data(data) {
    srand(static_cast<unsigned>(time(NULL)));

    // _backgroundTexture = new sf::Sprite();
    // _reloadLobbiesButton = new sf::Sprite();
    _titleText = new sf::Text();
    // _backgroundForLobbyEntries = new sf::RectangleShape();
    // _backgroundForLobbyEntriesPanel = new sf::RectangleShape();
    // _tittleNameLobby = new sf::Text();
    // _tittleNamePlayer = new sf::Text();
    // _tittleIpLobby = new sf::Text();
    // _spacer = new sf::RectangleShape();
}

void GameplayState::Init(){
    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }
    

    _data->assetManager.LoadTexture("background", "assets/background.jpg");
    // _backgroundTexture->setTexture(_data->assetManager.GetTexture("background"));
    _data->assetManager.LoadTexture("reloadLobbiesButton", "assets/flair_arrow_3.png");
    _data->assetManager.LoadTexture("reloadLobbiesButtonHover", "assets/flair_arrow_3_hover.png");

    _titleText->setFont(_font);
    _titleText->setString("Game");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(sf::Color::White);


    _titleText->setPosition(_data->window.getSize().x,60);


    // _backgroundForLobbyEntries->setSize(sf::Vector2f(950, 600));
    // _backgroundForLobbyEntries->setFillColor(sf::Color(58, 58, 58, 200));
    // _backgroundForLobbyEntries->setPosition(165, 200);
    //
    // _backgroundForLobbyEntriesPanel->setSize(sf::Vector2f(950,65));
    // _backgroundForLobbyEntriesPanel->setFillColor(sf::Color(88, 88, 88, 200));
    // _backgroundForLobbyEntriesPanel->setPosition(165, 200);
    //
    // _spacer->setSize(sf::Vector2f(950, 5));
    // _spacer->setFillColor(sf::Color(255, 255, 255, 200));
    // _spacer->setPosition(165, 265);
    //
    // _reloadLobbiesButton->setTexture(_data->assetManager.GetTexture("reloadLobbiesButton"));
    // _reloadLobbiesButton->setPosition(1000, 202);
    //
    // _tittleNameLobby->setFont(_font);
    // _tittleNameLobby->setString("Lobby Name");
    // _tittleNameLobby->setCharacterSize(23);
    // _tittleNameLobby->setFillColor(sf::Color::White);
    // _tittleNameLobby->setPosition(200, 218);
    //
    // _tittleNamePlayer->setFont(_font);
    // _tittleNamePlayer->setString("Player Name");
    // _tittleNamePlayer->setCharacterSize(23);
    // _tittleNamePlayer->setFillColor(sf::Color::White);
    // _tittleNamePlayer->setPosition(480, 218);
    //
    // _tittleIpLobby->setFont(_font);
    // _tittleIpLobby->setString("IP");
    // _tittleIpLobby->setCharacterSize(23);
    // _tittleIpLobby->setFillColor(sf::Color::White);
    // _tittleIpLobby->setPosition(800, 218);
   
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

    // Animation logic
    if(_animationState == AnimationState::ENTERING) {
        enteringAnimation();
    } else if(_animationState == AnimationState::EXITING) {
        exitingAnimation();
    } else {
        standartAnimation();
    }


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
    // if(_data->inputManager.IsSpriteHover(*_reloadLobbiesButton, _data->window)) {
    //     _reloadLobbiesButton->setTexture(_data->assetManager.GetTexture("reloadLobbiesButtonHover"));
    // } else {
    //     _reloadLobbiesButton->setTexture(_data->assetManager.GetTexture("reloadLobbiesButton"));
    // }

    return;
}




void GameplayState::Draw() {
    _data->window.clear();
    // _data->window.draw(*_backgroundTexture);
    // _data->window.draw(*_backgroundForLobbyEntries);
    // _data->window.draw(*_backgroundForLobbyEntriesPanel);
    // _data->window.draw(*_spacer);
    // _data->window.draw(*_tittleNameLobby);
    // _data->window.draw(*_tittleNamePlayer);
    // _data->window.draw(*_tittleIpLobby);
    // _data->window.draw(*_reloadLobbiesButton);
    _data->window.draw(*_titleText);

    // for(auto backgroundForOption : _backgroundForOption) {
    //     _data->window.draw(*backgroundForOption);
    // }
    //
    // for(auto entry : _lobbyEntries) {
    //     _data->window.draw(*entry);
    // }
    // for(auto entry : _playerEntries) {
    //     _data->window.draw(*entry);
    // }
    // for(auto entry : _ipEntries) {
    //     _data->window.draw(*entry);
    // }
    // for(auto button : _joinButtons) {
    //     _data->window.draw(*button);
    // }
    //
    // for(auto buttonText : _joinButtonsText) {
    //     _data->window.draw(*buttonText);
    // }


    _data->window.display();
}


GameplayState::~GameplayState() {
    // delete _backgroundTexture;
    // delete _backgroundForLobbyEntries;
    // delete _backgroundForLobbyEntriesPanel;
    // delete _spacer;
    // delete _tittleNameLobby;
    // delete _tittleNamePlayer;
    // delete _tittleIpLobby;
    // delete _reloadLobbiesButton;
    delete _titleText;
}


void GameplayState::ClearObjects() {
   // _data->assetManager.clearAssets();
    //_data->soundManager.ClearSounds();
}