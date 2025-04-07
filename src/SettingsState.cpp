#include "SettingsState.h"

SettingsState::SettingsState(GameDataRef data) : _data(data) {
    srand(static_cast<unsigned>(time(NULL)));

    _backgroundTexture = new sf::Sprite();
    _titleText = new sf::Text();
}

void SettingsState::Init(){
    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }

    _data->assetManager.LoadTexture("background", "assets/background.jpg");
    _backgroundTexture->setTexture(_data->assetManager.GetTexture("background"));

    _titleText->setFont(_font);
    _titleText->setString("Settings");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(sf::Color::White);


    _titleTargetPos = sf::Vector2f(500, 100);
    _titleText->setPosition(_data->window.getSize().x, _titleTargetPos.y);
   
}

void SettingsState::HandleInput() {
    sf::Event event;
    while (_data->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _data->window.close();
            return;
        }
        if(event.type == sf::Event::MouseButtonPressed ) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
                _data->stateManager.RemoveState();
            }
            return;
        }
    }
}

void SettingsState::Update() {
    sf::Vector2f currentPos = _titleText->getPosition();

    if (currentPos.x > _titleTargetPos.x) {


        currentPos.x -= _exitAnimationSpeed;
        if (currentPos.x < _titleTargetPos.x) currentPos.x = _titleTargetPos.x;

        _titleText->setPosition(currentPos);
    } else {

    }
}



void SettingsState::Draw() {
    _data->window.clear();
    _data->window.draw(*_backgroundTexture);
    _data->window.draw(*_titleText);

    _data->window.display();
}


SettingsState::~SettingsState() {
    delete _backgroundTexture;
    delete _titleText;
    
}


void SettingsState::ClearObjects() {
   // _data->assetManager.clearAssets();
    //_data->soundManager.ClearSounds();
}