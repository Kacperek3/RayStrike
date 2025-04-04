#include "MenuState.h"

MenuState::MenuState(GameDataRef data) : _data(data) {
    srand(static_cast<unsigned>(time(NULL)));

    _titleText = new sf::Text();
    _copyrightText = new sf::Text();
    _createGameButton = new sf::RectangleShape();
    _createGameButtonText = new sf::Text();
    _joinGameButton = new sf::RectangleShape();
    _joinGameButtonText = new sf::Text();
    _settingsButton = new sf::RectangleShape();
    _settingsButtonText = new sf::Text();
    _exitButton = new sf::RectangleShape();
    _exitButtonText = new sf::Text();

    _soundtrack = new sf::Sound();
    
    _bullet = new sf::Sprite();
}

void MenuState::Init(){
    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }

    _data->soundManager.LoadSoundBuffer("soundtrack", "assets/sounds/menuSounds/soundracks/GameSoundtrack.wav");
    _soundtrack->setBuffer(_data->soundManager.GetSoundBuffer("soundtrack"));
    _soundtrack->setLoop(true);
    _soundtrack->setVolume(20);
    _soundtrack->play();

    _titleText->setFont(_font);
    _titleText->setString("RayStrike");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(sf::Color::White);
    _titleText->setPosition(500, 100);


    _copyrightText->setFont(_font);
    _copyrightText->setString("Kacper & Piotr Studios 2025");
    _copyrightText->setCharacterSize(17);
    _copyrightText->setFillColor(sf::Color::White);
    _copyrightText->setPosition(70, 850);

    _createGameButton->setSize(sf::Vector2f(350, 50));
    _createGameButton->setFillColor(sf::Color::Blue);
    _createGameButton->setPosition(300, 300);

    _createGameButtonText->setFont(_font);
    _createGameButtonText->setString("Create Game");
    _createGameButtonText->setCharacterSize(30);
    _createGameButtonText->setFillColor(sf::Color::White);
    _createGameButtonText->setPosition(370, 308);


    _joinGameButton->setSize(sf::Vector2f(350, 50));
    _joinGameButton->setFillColor(sf::Color::Blue);
    _joinGameButton->setPosition(300, 390);

    _joinGameButtonText->setFont(_font);
    _joinGameButtonText->setString("Join Game");
    _joinGameButtonText->setCharacterSize(30);
    _joinGameButtonText->setFillColor(sf::Color::White);
    _joinGameButtonText->setPosition(370, 398);


    _settingsButton->setSize(sf::Vector2f(350, 50));
    _settingsButton->setFillColor(sf::Color::Blue);
    _settingsButton->setPosition(300, 480);

    _settingsButtonText->setFont(_font);
    _settingsButtonText->setString("Settings");
    _settingsButtonText->setCharacterSize(30);
    _settingsButtonText->setFillColor(sf::Color::White);
    _settingsButtonText->setPosition(370, 488);


    _exitButton->setSize(sf::Vector2f(350, 50));
    _exitButton->setFillColor(sf::Color::Blue);
    _exitButton->setPosition(300, 570);

    _exitButtonText->setFont(_font);
    _exitButtonText->setString("Exit");
    _exitButtonText->setCharacterSize(30);
    _exitButtonText->setFillColor(sf::Color::White);
    _exitButtonText->setPosition(370, 578);


    auto storeButtonData = [&](sf::RectangleShape* btn, sf::Text* txt) {
        sf::Vector2f originalBtnPos = btn->getPosition();
        sf::Vector2f originalTxtPos = txt->getPosition();
        sf::FloatRect originalBounds = btn->getGlobalBounds();
        sf::Color originalColor = btn->getFillColor();
        _buttonData[btn] = {originalBtnPos, originalTxtPos, originalBounds, originalColor};
    };

    storeButtonData(_createGameButton, _createGameButtonText);
    storeButtonData(_joinGameButton, _joinGameButtonText);
    storeButtonData(_settingsButton, _settingsButtonText);
    storeButtonData(_exitButton, _exitButtonText);



    _data->assetManager.LoadTexture("bullet", "assets/bullet.png");
    _bullet->setTexture(_data->assetManager.GetTexture("bullet"));
    _bullet->setScale(3.0f, 3.0f);

    _bulletSpeed = 8.0f;
    _windowSize = _data->window.getSize();
    
    // Początkowe ustawienie pocisku
    ResetBulletPositionAndVelocity();
}

void MenuState::HandleInput() {
    sf::Event event;
    while (_data->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _data->window.close();
            return;
        }
        if(event.type == sf::Event::MouseButtonPressed ) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
                if (_createGameButton->getGlobalBounds().contains(mousePos)) {
                    //_data->stateManager.AddState(StateRef(new CreateGameState(_data)));
                }
                else if (_joinGameButton->getGlobalBounds().contains(mousePos)) {
                    //_data->stateManager.AddState(StateRef(new JoinGameState(_data)));
                }
                else if (_settingsButton->getGlobalBounds().contains(mousePos)) {
                    //_data->stateManager.AddState(StateRef(new SettingsState(_data)));
                }
                else if (_exitButton->getGlobalBounds().contains(mousePos)) {
                    _data->window.close();
                    return;
                }
            }
            return;
        }
    }
}

void MenuState::Update() {
    sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
    const float hoverOffset = 10.f;
    const sf::Color hoverColor(0, 59, 190);

    for (auto& [button, data] : _buttonData) {
        auto& [originalBtnPos, originalTxtPos, originalBounds, originalColor] = data;
        sf::Text* text = nullptr;

        if (button == _createGameButton) text = _createGameButtonText;
        else if (button == _joinGameButton) text = _joinGameButtonText;
        else if (button == _settingsButton) text = _settingsButtonText;
        else if (button == _exitButton) text = _exitButtonText;

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


    _bullet->move(_bulletVelocity);

    
    sf::FloatRect bulletBounds = _bullet->getGlobalBounds();
    if (bulletBounds.left + bulletBounds.width < 0 || 
        bulletBounds.left > _windowSize.x ||
        bulletBounds.top + bulletBounds.height < 0 ||
        bulletBounds.top > _windowSize.y) {
        ResetBulletPositionAndVelocity();
    }

    float angle = atan2(_bulletVelocity.y, _bulletVelocity.x) * 180 / 3.14159265f;
    _bullet->setRotation(angle);
}


void MenuState::ResetBulletPositionAndVelocity() {

    int edge = rand() % 4;
    float x, y;

    switch (edge) {
        case 0:
            x = -_bullet->getGlobalBounds().width;
            y = static_cast<float>(rand() % _windowSize.y);
            break;
        case 1:
            x = static_cast<float>(_windowSize.x);
            y = static_cast<float>(rand() % _windowSize.y);
            break;
        case 2: 
            x = static_cast<float>(rand() % _windowSize.x);
            y = -_bullet->getGlobalBounds().height;
            break;
        case 3: 
            x = static_cast<float>(rand() % _windowSize.x);
            y = static_cast<float>(_windowSize.y);
            break;
    }

    _bullet->setPosition(x, y);

    sf::Vector2f target(
        static_cast<float>(rand() % _windowSize.x),
        static_cast<float>(rand() % _windowSize.y)
    );

    sf::Vector2f direction = target - _bullet->getPosition();
    float length = sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length != 0) {
        _bulletVelocity = (direction / length) * _bulletSpeed;
    }
}



void MenuState::Draw() {
    _data->window.clear();
    _data->window.draw(*_bullet);

    _data->window.draw(*_titleText);
    _data->window.draw(*_copyrightText);
    _data->window.draw(*_createGameButton);
    _data->window.draw(*_createGameButtonText);

    _data->window.draw(*_joinGameButton);
    _data->window.draw(*_joinGameButtonText);

    _data->window.draw(*_settingsButton);
    _data->window.draw(*_settingsButtonText);

    _data->window.draw(*_exitButton);
    _data->window.draw(*_exitButtonText);

    _data->window.display();
}


MenuState::~MenuState() {
    delete _bullet;
    delete _titleText;
    delete _copyrightText;
    delete _createGameButton;
    delete _createGameButtonText;

    delete _joinGameButton;
    delete _joinGameButtonText;

    delete _settingsButton;
    delete _settingsButtonText;

    delete _exitButton;
    delete _exitButtonText;

    delete _soundtrack;
}


void MenuState::ClearObjects() {
    _data->assetManager.clearAssets();
    _data->soundManager.ClearSounds();
}