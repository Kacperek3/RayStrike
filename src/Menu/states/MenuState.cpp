#include "MenuState.h"
#include "CreateLobbyState.h"
#include "JoinLobbyState.h"
#include "SettingsState.h"

MenuState::MenuState(GameDataRef data) : _data(data) {
    srand(static_cast<unsigned>(time(NULL)));
    _titleText = new sf::Text();
    _copyrightText = new sf::Text();
    _backgroundTexture = new sf::Sprite();
    _createGameButton = new sf::RectangleShape();
    _createGameButtonText = new sf::Text();
    _joinGameButton = new sf::RectangleShape();
    _joinGameButtonText = new sf::Text();
    _settingsButton = new sf::RectangleShape();
    _settingsButtonText = new sf::Text();
    _exitButton = new sf::RectangleShape();
    _exitButtonText = new sf::Text();

    _bulletSpeed = 8.0f;
    _windowSize = _data->window.getSize();
}

void MenuState::Init() {
    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }

    _data->assetManager.LoadTexture("background", "assets/background.jpg");
    _backgroundTexture->setTexture(_data->assetManager.GetTexture("background"));

    _data->soundManager.LoadSoundBuffer("soundtrack",
                                        "assets/sounds/menuSounds/soundracks/GameSoundtrack.wav");

    _data->menuSoundtrack.setBuffer(_data->soundManager.GetSoundBuffer("soundtrack"));
    _data->menuSoundtrack.setLoop(true);
    _data->menuSoundtrack.setVolume(20);
    _data->menuSoundtrack.play();

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
    _createGameButton->setFillColor(sf::Color(80, 150, 255, 150));
    _createGameButton->setPosition(300, 300);

    _createGameButtonText->setFont(_font);
    _createGameButtonText->setString("Create Game");
    _createGameButtonText->setCharacterSize(30);
    _createGameButtonText->setFillColor(sf::Color::White);
    _createGameButtonText->setPosition(370, 308);

    _joinGameButton->setSize(sf::Vector2f(350, 50));
    _joinGameButton->setFillColor(sf::Color(80, 150, 255, 150));
    _joinGameButton->setPosition(300, 390);

    _joinGameButtonText->setFont(_font);
    _joinGameButtonText->setString("Join Game");
    _joinGameButtonText->setCharacterSize(30);
    _joinGameButtonText->setFillColor(sf::Color::White);
    _joinGameButtonText->setPosition(370, 398);

    _settingsButton->setSize(sf::Vector2f(350, 50));
    _settingsButton->setFillColor(sf::Color(80, 150, 255, 150));
    _settingsButton->setPosition(300, 480);

    _settingsButtonText->setFont(_font);
    _settingsButtonText->setString("Settings");
    _settingsButtonText->setCharacterSize(30);
    _settingsButtonText->setFillColor(sf::Color::White);
    _settingsButtonText->setPosition(370, 488);

    _exitButton->setSize(sf::Vector2f(350, 50));
    _exitButton->setFillColor(sf::Color(80, 150, 255, 150));
    _exitButton->setPosition(300, 570);

    _exitButtonText->setFont(_font);
    _exitButtonText->setString("Exit");
    _exitButtonText->setCharacterSize(30);
    _exitButtonText->setFillColor(sf::Color::White);
    _exitButtonText->setPosition(370, 578);

    auto storeButtonData = [&](sf::RectangleShape *btn, sf::Text *txt) {
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

    _data->assetManager.LoadTexture("bulletBlue", "assets/bulletBlue.png");
    _data->assetManager.LoadTexture("bulletRed", "assets/bulletRed.png");
    _bullets.resize(5);
    for (auto &bullet : _bullets) {
        bool isRed = (rand() % 2 == 0);
        bullet.sprite.setTexture(
            _data->assetManager.GetTexture(isRed ? "bulletRed" : "bulletBlue"));
        bullet.sprite.setScale(3.0f, 3.0f);
        ResetBullet(bullet, true);
    }
}

void MenuState::HandleInput() {
    sf::Event event;
    while (_data->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _data->window.close();
            return;
        }
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
                if (_createGameButton->getGlobalBounds().contains(mousePos)) {
                    _animationState = AnimationState::EXITING;
                    _state = State::CREATE_GAME;
                } else if (_joinGameButton->getGlobalBounds().contains(mousePos)) {
                    _animationState = AnimationState::EXITING;
                    _state = State::JOIN_GAME;
                } else if (_settingsButton->getGlobalBounds().contains(mousePos)) {
                    _animationState = AnimationState::EXITING;
                    _state = State::SETTINGS;
                } else if (_exitButton->getGlobalBounds().contains(mousePos)) {
                    _data->window.close();
                    return;
                }
            }
            return;
        }
    }
}

void MenuState::Update() {
    for (auto &bullet : _bullets) {
        bullet.sprite.move(bullet.velocity);

        sf::FloatRect bounds = bullet.sprite.getGlobalBounds();
        if (bounds.left + bounds.width < 0 || bounds.left > _windowSize.x ||
            bounds.top + bounds.height < 0 || bounds.top > _windowSize.y) {
            ResetBullet(bullet);
        }

        float angle = atan2(bullet.velocity.y, bullet.velocity.x) * 180 / 3.14159265f;
        bullet.sprite.setRotation(angle);
    }

    if (_animationState == AnimationState::EXITING) {
        exitingAnimation();
        return;
    } else if (_animationState == AnimationState::ENTERING) {
        enteringAnimation();
        return;
    } else if (_animationState == AnimationState::NONE) {
        standartAnimation();
        return;
    }
}

void MenuState::exitingAnimation() {
    bool allButtonsOffScreen = true;

    for (auto &[button, data] : _buttonData) {
        auto &[originalBtnPos, originalTxtPos, originalBounds, originalColor] = data;

        sf::Vector2f newPos = button->getPosition();
        newPos.x -= _exitAnimationSpeed;
        button->setPosition(newPos);
        _titleText->setPosition(_titleText->getPosition().x - _exitAnimationSpeed,
                                _titleText->getPosition().y);

        sf::Text *text = nullptr;
        if (button == _createGameButton)
            text = _createGameButtonText;
        else if (button == _joinGameButton)
            text = _joinGameButtonText;
        else if (button == _settingsButton)
            text = _settingsButtonText;
        else if (button == _exitButton)
            text = _exitButtonText;

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

    if (allButtonsOffScreen) {
        std::cout << "All buttons are off screen, exiting to settings." << std::endl;

        _animationState = AnimationState::ENTERING;

        if (_state == State::CREATE_GAME) {
            _data->stateManager.AddState(StateRef(new CreateLobbyState(_data)), false);
        } else if (_state == State::SETTINGS) {
            _data->stateManager.AddState(StateRef(new SettingsState(_data)), false);
        } else if (_state == State::JOIN_GAME) {
            _data->stateManager.AddState(StateRef(new JoinLobbyState(_data)), false);
        }
    }
}
void MenuState::enteringAnimation() {
    for (auto &[button, data] : _buttonData) {
        auto &[originalBtnPos, originalTxtPos, originalBounds, originalColor] = data;

        // Pobierz aktualną pozycję przycisku
        sf::Vector2f newPos = button->getPosition();

        // Jeśli przycisk jest poza ekranem (x < 300), przesuwaj go w prawo
        if (newPos.x < 300) {
            newPos.x += _exitAnimationSpeed; // Przesuwaj w prawo
            if (newPos.x > 300) {
                newPos.x = 300; // Zatrzymaj na pozycji docelowej
            }
            button->setPosition(newPos);
        }

        sf::Text *text = nullptr;
        if (button == _createGameButton)
            text = _createGameButtonText;
        else if (button == _joinGameButton)
            text = _joinGameButtonText;
        else if (button == _settingsButton)
            text = _settingsButtonText;
        else if (button == _exitButton)
            text = _exitButtonText;

        if (text) {
            sf::Vector2f textPos = text->getPosition();
            if (textPos.x < 300) {
                textPos.x += _exitAnimationSpeed;
                if (textPos.x > 300) {
                    textPos.x = 300;
                }
                text->setPosition(textPos);
            }
        }
        sf::FloatRect bounds = button->getGlobalBounds();
    }
    if (_titleText->getPosition().x < 0) {
        _titleText->setPosition(0, 100); // Start poza ekranem
    }

    sf::Vector2f currentPos = _titleText->getPosition();

    if (currentPos.x < 500) {
        currentPos.x += _exitAnimationSpeed;
        if (currentPos.x >= 500) {
            currentPos.x = 500;
            _animationState = AnimationState::NONE;
        }
    }

    _titleText->setPosition(currentPos);
}
void MenuState::standartAnimation() {
    sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
    const float hoverOffset = 10.f;
    const sf::Color hoverColor(0, 59, 190);
    for (auto &[button, data] : _buttonData) {
        auto &[originalBtnPos, originalTxtPos, originalBounds, originalColor] = data;
        sf::Text *text = nullptr;

        if (button == _createGameButton)
            text = _createGameButtonText;
        else if (button == _joinGameButton)
            text = _joinGameButtonText;
        else if (button == _settingsButton)
            text = _settingsButtonText;
        else if (button == _exitButton)
            text = _exitButtonText;

        if (originalBounds.contains(mousePos)) {
            button->setPosition(originalBtnPos.x + hoverOffset, originalBtnPos.y);
            button->setFillColor(hoverColor);
            if (text)
                text->setPosition(originalTxtPos.x + hoverOffset, originalTxtPos.y);
        } else {
            button->setPosition(originalBtnPos);
            button->setFillColor(originalColor);
            if (text)
                text->setPosition(originalTxtPos);
        }
    }
}

void MenuState::ResetBullet(Bullet &bullet, bool initialSpawn) {
    const int MAX_BULLETS = 6;
    const float LINE_SPACING = 50.0f;

    bool isRed = (rand() % 2 == 0);
    bullet.sprite.setTexture(_data->assetManager.GetTexture(isRed ? "bulletRed" : "bulletBlue"));
    // Losowa krawędź
    int edge = initialSpawn ? rand() % 4 : rand() % 2 ? 0 : 1;

    sf::Vector2f startPos;
    switch (edge) {
    case 0: // Lewa
        startPos = {-100.f, static_cast<float>(rand() % _windowSize.y)};
        break;
    case 1: // Prawa
        startPos = {static_cast<float>(_windowSize.x + 100),
                    static_cast<float>(rand() % _windowSize.y)};
        break;
    case 2: // Góra
        startPos = {static_cast<float>(rand() % _windowSize.x), -100.f};
        break;
    case 3: // Dół
        startPos = {static_cast<float>(rand() % _windowSize.x),
                    static_cast<float>(_windowSize.y + 100)};
        break;
    }

    // Cel w przeciwnej połowie ekranu
    sf::Vector2f target;
    if (edge == 0 || edge == 1) { // Poziomy ruch
        target = {edge == 0 ? static_cast<float>(_windowSize.x + 100) : -100.f,
                  static_cast<float>(rand() % _windowSize.y)};
    } else { // Pionowy ruch
        target = {static_cast<float>(rand() % _windowSize.x),
                  edge == 2 ? static_cast<float>(_windowSize.y + 100) : -100.f};
    }

    // Kierunek ruchu
    sf::Vector2f direction = target - startPos;
    float length = sqrt(direction.x * direction.x + direction.y * direction.y);
    bullet.velocity = (direction / length) * _bulletSpeed;
    bullet.sprite.setPosition(startPos);

    // Losowa szansa na grupę
    if (!initialSpawn && (rand() % 100) < (_groupSpawnChance * 100)) {
        int groupSize = 2 + rand() % (_maxGroupSize - 1);
        bool isLineFormation = (rand() % 2 == 0); // 50% szans na formację liniową

        for (int i = 0; i < groupSize; i++) {
            if (_bullets.size() >= MAX_BULLETS)
                break;

            Bullet newBullet;
            newBullet.sprite.setTexture(
                _data->assetManager.GetTexture((rand() % 2 == 0) ? "bulletRed" : "bulletBlue"));
            newBullet.sprite.setScale(3.0f, 3.0f);

            if (isLineFormation) {
                // Formacja liniowa - pociski lecą jeden za drugim
                sf::Vector2f offset = (-direction) * (LINE_SPACING * static_cast<float>(i + 1));
                newBullet.sprite.setPosition(startPos + offset);
                newBullet.velocity = direction * _bulletSpeed;
            } else {
                // Standardowa formacja z losowym kątem
                float angleVariation = (rand() % 30 - 15) * 3.14159265f / 180.f;
                sf::Vector2f groupDirection = {
                    direction.x * cos(angleVariation) - direction.y * sin(angleVariation),
                    direction.x * sin(angleVariation) + direction.y * cos(angleVariation)};
                groupDirection = groupDirection / length * _bulletSpeed;
                newBullet.velocity = groupDirection;
                newBullet.sprite.setPosition(startPos);
            }

            newBullet.sprite.setRotation(atan2(newBullet.velocity.y, newBullet.velocity.x) * 180 /
                                         3.14159265f);
            newBullet.isGroupBullet = true;

            _bullets.push_back(newBullet);
        }
    }
}

void MenuState::Draw() {
    _data->window.clear();
    _data->window.draw(*_backgroundTexture);
    for (const auto &bullet : _bullets) {
        _data->window.draw(bullet.sprite);
    }

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
    _bullets.clear();
    delete _backgroundTexture;
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
}

void MenuState::ClearObjects() {
    //_data->assetManager.clearAssets();
    //  _data->soundManager.ClearSounds();
}
