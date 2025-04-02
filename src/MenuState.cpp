#include "MenuState.h"

MenuState::MenuState(GameDataRef data) : _data(data) {
    _titleText = new sf::Text();
    _playButton = new sf::RectangleShape();
    _playButtonText = new sf::Text();
}

void MenuState::Init(){
    if (!_font.loadFromFile("assets/fonts/Poppins-Black.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }

    _titleText->setFont(_font);
    _titleText->setString("RayStrike");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(sf::Color::White);
    _titleText->setPosition(300, 100);

    _playButton->setSize(sf::Vector2f(200, 50));
    _playButton->setFillColor(sf::Color::Blue);
    _playButton->setPosition(300, 300);

    _playButtonText->setFont(_font);
    _playButtonText->setString("Play");
    _playButtonText->setCharacterSize(30);
    _playButtonText->setFillColor(sf::Color::White);
    _playButtonText->setPosition(370, 310);
}

void MenuState::HandleInput() {
    sf::Event event;
    while (_data->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _data->window.close();
            return;
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
            
        }
    }
}

void MenuState::Update() {
    
}


void MenuState::Draw() {
    _data->window.clear();

    _data->window.draw(*_titleText);
    _data->window.draw(*_playButton);
    _data->window.draw(*_playButtonText);

    _data->window.display();
}


MenuState::~MenuState() {
    delete _titleText;
    delete _playButton;
    delete _playButtonText;
}


void MenuState::ClearObjects() {
    _data->assetManager.clearAssets();
}