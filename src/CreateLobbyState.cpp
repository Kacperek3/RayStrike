#include "CreateLobbyState.h"
#include "LobbyWaitingState.h"

CreateLobbyState::CreateLobbyState(GameDataRef data) : _data(data) {
    srand(static_cast<unsigned>(time(NULL)));

    _backgroundTexture = new sf::Sprite();
    _titleText = new sf::Text();

    _backButton = new sf::RectangleShape();
    _backButtonText = new sf::Text();
    _createLobbyButton = new sf::RectangleShape();
    _createLobbyButtonText = new sf::Text();

    _lobbyNameTextField = nullptr;

    _tesseract = new Tesseract();
}

void CreateLobbyState::Init(){
    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }


    _lobbyNameTextField = new LabeledTextField(760, 280, 400, 50, 140, "LOBBY NAME", _font);
    _lobbyPlayerNameTextField = new LabeledTextField(760, 400, 400, 50, 160, "PLAYER NAME", _font);

    _data->assetManager.LoadTexture("background", "assets/background.jpg");
    _backgroundTexture->setTexture(_data->assetManager.GetTexture("background"));

    _titleText->setFont(_font);
    _titleText->setString("Create Lobby");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(sf::Color::White);
    _titleText->setPosition(_data->window.getSize().x,100);

    _backButton->setSize(sf::Vector2f(250, 50));
    _backButton->setFillColor(sf::Color(80, 150, 255,150));
    _backButton->setPosition(sf::Vector2f(300, 700));

    _backButtonText->setFont(_font);
    _backButtonText->setString("Back");
    _backButtonText->setCharacterSize(30);
    _backButtonText->setFillColor(sf::Color::White);
    _backButtonText->setPosition(_backButton->getPosition().x + 70, _backButton->getPosition().y + 8);

    _createLobbyButton->setSize(sf::Vector2f(280, 50));
    _createLobbyButton->setFillColor(sf::Color(80, 150, 255,150));
    _createLobbyButton->setPosition(sf::Vector2f(750, 700));

    _createLobbyButtonText->setFont(_font);
    _createLobbyButtonText->setString("Create Lobby");
    _createLobbyButtonText->setCharacterSize(30);
    _createLobbyButtonText->setFillColor(sf::Color::White);
    _createLobbyButtonText->setPosition(_createLobbyButton->getPosition().x + 40, _createLobbyButton->getPosition().y + 8);


    _tesseract->setPosition(sf::Vector2f(620, 400));
    _tesseract->setScale(3.0f);
 

    auto storeButtonData = [&](sf::RectangleShape* btn, sf::Text* txt) {
        sf::Vector2f originalBtnPos = btn->getPosition();
        sf::Vector2f originalTxtPos = txt->getPosition();
        sf::FloatRect originalBounds = btn->getGlobalBounds();
        sf::Color originalColor = btn->getFillColor();
        _buttonData[btn] = {originalBtnPos, originalTxtPos, originalBounds, originalColor};
    };
    storeButtonData(_backButton, _backButtonText);  
    storeButtonData(_createLobbyButton, _createLobbyButtonText);

    _backButton->setPosition(1100, 700);
    _backButtonText->setPosition(1170, 708);

    _createLobbyButton->setPosition(1100, 700);
    _createLobbyButtonText->setPosition(1100, 708);
}

void CreateLobbyState::HandleInput() {
    sf::Event event;
    while (_data->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _data->window.close();
            return;
        }
        _lobbyNameTextField->handleEvent(event);
        _lobbyPlayerNameTextField->handleEvent(event);
        if(event.type == sf::Event::MouseButtonPressed ) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
                if (_backButton->getGlobalBounds().contains(mousePos)) {
                    _animationState = AnimationState::EXITING;
                }
                else if( _createLobbyButton->getGlobalBounds().contains(mousePos)) {
                    std::string lobbyName = _lobbyNameTextField->getText();
                    std::string playerName = _lobbyPlayerNameTextField->getText();

                    if (lobbyName.empty() || playerName.empty()) {
                        std::cout << "Please fill in all fields." << std::endl;
                    } else {
                        _data->stateManager.AddState(StateRef(new LobbyWaitingState(_data, lobbyName, playerName)), false);
                    }
                }
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

    _tesseract->update();
}


void CreateLobbyState::exitingAnimation() {
    
    bool allButtonsOffScreen = false;

    for (auto& [button, data] : _buttonData) {
        auto& [originalBtnPos, originalTxtPos, originalBounds, originalColor] = data;
        
        sf::Vector2f newPos = button->getPosition();
        newPos.x += _exitAnimationSpeed;
        button->setPosition(newPos);
        _titleText->setPosition(_titleText->getPosition().x + _exitAnimationSpeed, _titleText->getPosition().y);


        sf::Text* text = nullptr;
        if (button == _backButton) text = _backButtonText;
        else if (button == _createLobbyButton) text = _createLobbyButtonText;
        
        if (text) {
            sf::Vector2f textPos = text->getPosition();
            textPos.x += _exitAnimationSpeed;
            text->setPosition(textPos);
        }
        if (newPos.x >= _data->window.getSize().x) {
            allButtonsOffScreen = true;
        }
    }

    _lobbyNameTextField->move(sf::Vector2f(_exitAnimationSpeed, 0));
    _lobbyPlayerNameTextField->move(sf::Vector2f(_exitAnimationSpeed, 0));

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

void CreateLobbyState::enteringAnimation() {
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
        else if (button == _createLobbyButton) text = _createLobbyButtonText;
        
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


    
    _lobbyNameTextField->move(sf::Vector2f(-_exitAnimationSpeed, 0));
    _lobbyPlayerNameTextField->move(sf::Vector2f(-_exitAnimationSpeed, 0));

    sf::Vector2f currentPos = _titleText->getPosition();

    if (currentPos.x > 0) {
        currentPos.x -= _exitAnimationSpeed;
        if (currentPos.x <= 450){
            currentPos.x = 450;
            _animationState = AnimationState::NONE;
        }
        
        _titleText->setPosition(currentPos);

    }
}

void CreateLobbyState::standartAnimation(){
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




void CreateLobbyState::Draw() {
    _data->window.clear();
    _data->window.draw(*_backgroundTexture);
    
    _data->window.draw(*_titleText);

    _lobbyNameTextField->draw(_data->window);
    _lobbyPlayerNameTextField->draw(_data->window);

    _data->window.draw(*_backButton);
    _data->window.draw(*_backButtonText);
    _data->window.draw(*_createLobbyButton);
    _data->window.draw(*_createLobbyButtonText);

    _tesseract->draw(_data->window);
    _data->window.display();
}


CreateLobbyState::~CreateLobbyState() {
    delete _backgroundTexture;
    delete _tesseract;
    delete _titleText;
    delete _backButton;
    delete _backButtonText;
    delete _createLobbyButton;
    delete _createLobbyButtonText;
    delete _lobbyNameTextField;
    delete _lobbyPlayerNameTextField;
    
}


void CreateLobbyState::ClearObjects() {
   // _data->assetManager.clearAssets();
    //_data->soundManager.ClearSounds();
}