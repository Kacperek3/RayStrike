#include "CreateLobbyState.h"

CreateLobbyState::CreateLobbyState(GameDataRef data) : _data(data) {
    srand(static_cast<unsigned>(time(NULL)));

    _backgroundTexture = new sf::Sprite();
    _titleText = new sf::Text();

    _backButton = new sf::RectangleShape();
    _backButtonText = new sf::Text();

    _lobbyNameTextField = nullptr;
    _lobbyNameTextFieldHeader = new sf::Text();
    _lobbyNameTextFieldHeaderBox = new sf::RectangleShape();

}

void CreateLobbyState::Init(){
    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }
    _lobbyNameTextField = new TextField(450, 300, 400, 50, _font);
    _lobbyNameTextFieldHeader->setFont(_font);
    _lobbyNameTextFieldHeader->setString("LOBBY NAME");
    _lobbyNameTextFieldHeader->setCharacterSize(15);
    _lobbyNameTextFieldHeader->setFillColor(sf::Color::White);
    _lobbyNameTextFieldHeader->setPosition(456, 271);
    _lobbyNameTextFieldHeaderBox->setSize(sf::Vector2f(140, 60));
    _lobbyNameTextFieldHeaderBox->setFillColor(sf::Color(80, 150, 255,150));
    _lobbyNameTextFieldHeaderBox->setPosition(448, 266);


    _data->assetManager.LoadTexture("background", "assets/background.jpg");
    _backgroundTexture->setTexture(_data->assetManager.GetTexture("background"));

    _titleText->setFont(_font);
    _titleText->setString("Create Lobby");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(sf::Color::White);
    _titleText->setPosition(_data->window.getSize().x,100);

    _backButton->setSize(sf::Vector2f(250, 50));
    _backButton->setFillColor(sf::Color(80, 150, 255,150));
    _backButton->setPosition(300, 700);

    _backButtonText->setFont(_font);
    _backButtonText->setString("Back");
    _backButtonText->setCharacterSize(30);
    _backButtonText->setFillColor(sf::Color::White);
    _backButtonText->setPosition(370, 708);

    auto storeButtonData = [&](sf::RectangleShape* btn, sf::Text* txt) {
        sf::Vector2f originalBtnPos = sf::Vector2f(300, 700);
        sf::Vector2f originalTxtPos = sf::Vector2f(370, 708);
        sf::FloatRect originalBounds = btn->getGlobalBounds();
        sf::Color originalColor = btn->getFillColor();
        _buttonData[btn] = {originalBtnPos, originalTxtPos, originalBounds, originalColor};
    };
    storeButtonData(_backButton, _backButtonText);  

    _backButton->setPosition(1100, 700);
    _backButtonText->setPosition(1170, 708);
}

void CreateLobbyState::HandleInput() {
    sf::Event event;
    while (_data->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _data->window.close();
            return;
        }
        _lobbyNameTextField->handleEvent(event);
        if(event.type == sf::Event::MouseButtonPressed ) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
                if (_backButton->getGlobalBounds().contains(mousePos)) {
                    _animationState = AnimationState::EXITING;
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

void CreateLobbyState::enteringAnimation() {
    bool allButtonsOffScreen = true;

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
        if (currentPos.x <= 500){
            currentPos.x = 500;
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
            button->setPosition(originalBtnPos.x + hoverOffset, originalBtnPos.y);
            button->setFillColor(hoverColor);
            if (text) text->setPosition(originalTxtPos.x + hoverOffset, originalTxtPos.y);
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

    _data->window.draw(*_lobbyNameTextFieldHeaderBox);
    _lobbyNameTextField->draw(_data->window);
    _data->window.draw(*_lobbyNameTextFieldHeader);

    _data->window.draw(*_backButton);
    _data->window.draw(*_backButtonText);

    _data->window.display();
}


CreateLobbyState::~CreateLobbyState() {
    delete _backgroundTexture;
    delete _titleText;
    delete _backButton;
    delete _backButtonText;
    delete _lobbyNameTextField;
    delete _lobbyNameTextFieldHeader;
    delete _lobbyNameTextFieldHeaderBox;
}


void CreateLobbyState::ClearObjects() {
   // _data->assetManager.clearAssets();
    //_data->soundManager.ClearSounds();
}