#include "LobbyState.h"

LobbyState::LobbyState(const LobbyConfig config) : _config(config), _data(config.data) {
    srand(static_cast<unsigned>(time(NULL)));

    _backgroundTexture = new sf::Sprite();
    _crownIcon = new sf::Sprite();

    _backgroundPlayerList = new sf::RectangleShape();
    _backgroundPlayerListPanel = new sf::RectangleShape();
    _spacer = new sf::RectangleShape();
    _tittlePlayerList = new sf::Text();

    _titleText = new sf::Text();

    _backButton = new sf::RectangleShape();
    _backButtonText = new sf::Text();
    
    _networkLobbyManager = new NetworkLobbyManager(_config.serverSocketForClient, _config.clientSocket);
}

void LobbyState::Init(){
    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }



    _data->assetManager.LoadTexture("background", "assets/background.jpg");
    _backgroundTexture->setTexture(_data->assetManager.GetTexture("background"));
    _backgroundTexture->setPosition(0, 0);

    _data->assetManager.LoadTexture("crownIcon", "assets/crown.jpg");
    _crownIcon->setTexture(_data->assetManager.GetTexture("crownIcon"));
    _crownIcon->setPosition(1000, 60);

    _backgroundPlayerList->setSize(sf::Vector2f(400, 450));
    _backgroundPlayerList->setFillColor(sf::Color(58, 58, 58, 200));
    _backgroundPlayerList->setPosition(780, 170);
    _backgroundPlayerListPanel->setSize(sf::Vector2f(400, 65));
    _backgroundPlayerListPanel->setFillColor(sf::Color(88, 88, 88, 200));
    _backgroundPlayerListPanel->setPosition(780, 170);
    _spacer->setSize(sf::Vector2f(400, 5));
    _spacer->setFillColor(sf::Color(255, 255, 255, 200));
    _spacer->setPosition(780, 235);
    _tittlePlayerList->setFont(_font);
    _tittlePlayerList->setString("Player List");
    _tittlePlayerList->setCharacterSize(23);
    _tittlePlayerList->setFillColor(sf::Color::White);
    _tittlePlayerList->setPosition(900, 188);



    _titleText->setFont(_font);
    _titleText->setString("Lobby");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(sf::Color::White);
    _titleText->setPosition(_data->window.getSize().x,60);

    _backButton->setSize(sf::Vector2f(250, 50));
    _backButton->setFillColor(sf::Color(80, 150, 255,150));
    _backButton->setPosition(sf::Vector2f(300, 700));

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

}

void LobbyState::HandleInput() {
    sf::Event event;
    while (_data->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _data->window.close();
            return;
        }
        if(event.type == sf::Event::MouseButtonPressed ) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
                if (_backButton->getGlobalBounds().contains(mousePos)) {
                    _animationState = AnimationState::EXITING;
                    _networkLobbyManager->Send("__DISCONNECT__");
                }
            }
            return;
        }
    }
}

void LobbyState::Update() {
    // Animation logic
    if(_animationState == AnimationState::ENTERING) {
        enteringAnimation();
    } else if(_animationState == AnimationState::EXITING) {
        exitingAnimation();
    } else {
        standartAnimation();
    }

    std::string msg;
    while (_networkLobbyManager->WaitForMessage(msg, 0)) { // timeout 0 = nie blokuj
        if (msg.find("__DISCONNECT") != std::string::npos) {
            _animationState = AnimationState::EXITING;
            break;
        }
    }


    
}


void LobbyState::exitingAnimation() {
    
    bool allButtonsOffScreen = false;

    for (auto& [button, data] : _buttonData) {
        auto& [originalBtnPos, originalTxtPos, originalBounds, originalColor] = data;
        
        sf::Vector2f newPos = button->getPosition();
        newPos.x += _exitAnimationSpeed;
        button->setPosition(newPos);
        _titleText->setPosition(_titleText->getPosition().x + _exitAnimationSpeed, _titleText->getPosition().y);


        sf::Text* text = nullptr;
        if (button == _backButton) text = _backButtonText;
        
        if (text) {
            sf::Vector2f textPos = text->getPosition();
            textPos.x += _exitAnimationSpeed;
            text->setPosition(textPos);
        }
        if (newPos.x >= _data->window.getSize().x) {
            allButtonsOffScreen = true;
        }
    }

    sf::Vector2f currentPos = _titleText->getPosition();

    if (currentPos.x < _data->window.getSize().x) {
        currentPos.x += _exitAnimationSpeed;
        _titleText->setPosition(currentPos);
    }
    if (currentPos.x >= _data->window.getSize().x && allButtonsOffScreen) {
        currentPos.x = _data->window.getSize().x;
        _animationState = AnimationState::ENTERING;
        _data->stateManager.RemoveState();
    }
    
    return;
}

void LobbyState::enteringAnimation() {
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


    
    sf::Vector2f currentPos = _titleText->getPosition();

    if (currentPos.x > 0) {
        currentPos.x -= _exitAnimationSpeed;
        if (currentPos.x <= 550){
            currentPos.x = 550;
            _animationState = AnimationState::NONE;
        }
        
        _titleText->setPosition(currentPos);

    }
}

void LobbyState::standartAnimation(){
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


void LobbyState::Draw() {
    _data->window.clear();
    _data->window.draw(*_backgroundTexture);
    _data->window.draw(*_backgroundPlayerList);
    _data->window.draw(*_backgroundPlayerListPanel);
    _data->window.draw(*_spacer);
    _data->window.draw(*_tittlePlayerList);
    _data->window.draw(*_titleText);
    _data->window.draw(*_backButton);
    _data->window.draw(*_backButtonText);

    
    _data->window.display();
}


LobbyState::~LobbyState() {
    delete _backgroundTexture;
    delete _crownIcon;
    delete _backgroundPlayerList;
    delete _backgroundPlayerListPanel;
    delete _spacer;
    delete _tittlePlayerList;
    delete _titleText;
    delete _backButton;
    delete _backButtonText;
    delete _networkLobbyManager;
}


void LobbyState::ClearObjects() {
   // _data->assetManager.clearAssets();
    //_data->soundManager.ClearSounds();
}