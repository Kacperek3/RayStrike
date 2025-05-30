#include "GameplayStateGuest.h"

#include <cmath>
#include <iostream> // Required for std::cerr, std::cout
#include <string>   // Required for std::string, std::stof, std::stoi
#include <stdexcept> // Required for std::invalid_argument, std::out_of_range
#include <chrono> // Required for std::chrono::milliseconds

#define PI 3.14159f

GameplayStateGuest::GameplayStateGuest(GameDataRef data, int tcpSocketClient) : _data(data), _tcpSocketClient(tcpSocketClient), _running(true) {
    _windowSize = _data->window.getSize();
    _data->window.setMouseCursorVisible(false);

    // Initialize UdpNetworkManager for Guest
    

    // Initialize render objects (similar to GameplayStateHost)
    _player.render.indicatorText = new sf::Text();
    _enemy.render.indicatorText = new sf::Text();
    _roundOverText = new sf::Text();
    _restartText = new sf::Text();
    _player.render.bodySprite = new sf::Sprite();
    _player.render.gunSprite = new sf::Sprite();
    _enemy.render.bodySprite = new sf::Sprite();
    _enemy.render.gunSprite = new sf::Sprite();

    _udpManager = new UdpNetworkManager(-1, _tcpSocketClient); // tcpSocketServer is -1 for client
    if (!_udpManager->Initialize()) {
        std::cerr << "Guest: Failed to initialize UdpNetworkManager. TCP Socket: " << _tcpSocketClient << std::endl;
        // Handle this critical error. For now, logging.
        // This could be the source of the hang if Initialize() blocks,
        // or a crash if it returns false and _udpManager is used later without being valid.
    }

    // Start the network thread
    _networkThread = std::thread(&GameplayStateGuest::ReceiveNetworkData, this);
}

void GameplayStateGuest::RoundInit() {

    _player.core.hitbox.setPosition(_windowSize.x - 100, _windowSize.y / 2);
    _enemy.core.hitbox.setPosition(0 + 100, _windowSize.y / 2);

    _player.core.health = 100;
    _enemy.core.health = 100;
}


void GameplayStateGuest::InitPlayer(Player &p, float x, float y) {
    p.render.bodySprite->setOrigin(p.render.bodySprite->getLocalBounds().width/2, p.render.bodySprite->getLocalBounds().height/2);
    p.render.bodySprite->setPosition(x, y);
    p.render.bodySprite->setScale(2.0f, 2.0f);

    sf::FloatRect playerBounds = p.render.bodySprite->getGlobalBounds();

    p.core.hitbox.setRadius(playerBounds.width/2);
    p.core.hitbox.setOrigin(playerBounds.width/2, playerBounds.height/2);
    p.core.hitbox.setPosition(p.render.bodySprite->getPosition());
    p.core.hitbox.setFillColor(sf::Color::Transparent);
    p.core.hitbox.setOutlineColor(sf::Color::Green);
    p.core.hitbox.setOutlineThickness(2.0f);

    p.render.gunSprite->setOrigin(5.f, p.render.gunSprite->getLocalBounds().height/2);
    p.render.gunSprite->setPosition(p.render.bodySprite->getPosition());

    p.render.indicatorText->setFont(_font);
    p.render.indicatorText->setCharacterSize(14);
    p.render.indicatorText->setFillColor(sf::Color::White);
    p.render.indicatorText->setPosition(_windowSize.x/2, _windowSize.y/2);

    p.render.healthBarBackground.setSize(sf::Vector2f(60.f, 8.f));
    p.render.healthBarBackground.setFillColor(sf::Color(50, 50, 50));
    p.render.healthBarFill.setSize(sf::Vector2f(56.f, 6.f));
    p.render.healthBarFill.setFillColor(sf::Color(200, 30, 30));

}


void GameplayStateGuest::Init() {
    _data->assetManager.LoadTexture("pgun", "assets/pgun2.png");
    _data->assetManager.LoadTexture("egun", "assets/egun2.png");
    _data->assetManager.LoadTexture("player", "assets/player1.png");
    _data->assetManager.LoadTexture("enemy", "assets/enemy1.png");
    _data->assetManager.LoadTexture("bulletBlue", "assets/bulletBlue.png");

    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }

    _player.render.bodySprite->setTexture(_data->assetManager.GetTexture("player"));
    _player.render.gunSprite->setTexture(_data->assetManager.GetTexture("pgun"));
    _player.render.indicatorText->setString("You");
    InitPlayer(_player, _windowSize.x - 100, _windowSize.y / 2);

    _enemy.render.bodySprite->setTexture(_data->assetManager.GetTexture("enemy"));
    _enemy.render.gunSprite->setTexture(_data->assetManager.GetTexture("egun"));
    _enemy.render.indicatorText->setString("Enemy");
    InitPlayer(_enemy, 0 + 100, _windowSize.y / 2);

    _windowSize = _data->window.getSize();

    _roundOverText->setFont(_font);
    _roundOverText->setString("Round Over");
    _roundOverText->setCharacterSize(44);
    _roundOverText->setFillColor(sf::Color::White);
    sf::FloatRect textBounds = _roundOverText->getGlobalBounds();
    float textX = _windowSize.x / 2 - (textBounds.width / 2);
    float textY = 0 + 20 + textBounds.height;
    _roundOverText->setPosition(textX, textY);

    _restartText->setFont(_font);
    _restartText->setString("Press R to restart");
    _restartText->setCharacterSize(44);
    _restartText->setFillColor(sf::Color::White);
    textBounds = _restartText->getGlobalBounds();
    textX = _windowSize.x / 2 - (textBounds.width / 2);
    textY = textY + 50;
    _restartText->setPosition(textX, textY);

    _crosshair.dot.setFillColor(sf::Color::Red);
    _crosshair.dot.setRadius(1.0f);
    _crosshair.line.setFillColor(sf::Color::Green);
    _crosshair.line.setSize(sf::Vector2f(5.0f, 10.0f));
}


void GameplayStateGuest::HandleInput() {
    sf::Event event;
    while (_data->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _running = false; // Signal network thread to stop
            _data->window.close();
            if (_udpManager) _udpManager->Send("disconnect"); // Informuj hosta o rozłączeniu
        }

        if (event.type == sf::Event::KeyPressed) {
           if (event.key.code == sf::Keyboard::H) {
                _hitboxVisibility = !_hitboxVisibility;
           }
        }

        if (CheckWin()) {
            break;
        }
        if (event.type == sf::Event::MouseMoved) {
            _mousePosition = _data->inputManager.GetMousePosition(_data->window);
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            FireBullet(_player.render.bodySprite, _player.render.gunSprite);
            // Wysłanie informacji o wystrzale do hosta
            if (_udpManager) {
                // Serializacja danych pocisku (pozycja, prędkość)
                // Przykład: "fire;x;y;velX;velY"
                // Tutaj uproszczone, wyślij tylko fakt wystrzału
                _udpManager->Send("fire"); 
            }
        }
    }

    _player.core.velocity = {0, 0};

    if (CheckWin()) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
            RoundInit();
            if (_udpManager) _udpManager->Send("restart_request");
        }
        return;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) _player.core.velocity.y = -_player.core.speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) _player.core.velocity.y = _player.core.speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) _player.core.velocity.x = -_player.core.speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) _player.core.velocity.x = _player.core.speed;

    // Wysłanie pozycji gracza do hosta
    if (_udpManager && (_player.core.velocity.x != 0 || _player.core.velocity.y != 0)) {
        std::string posMsg = "pos;" + std::to_string(_player.core.hitbox.getPosition().x) + ";" + std::to_string(_player.core.hitbox.getPosition().y);
        _udpManager->Send(posMsg);
    }
}

void GameplayStateGuest::Update() {
    // Aktualizacja pozycji gracza
    _player.core.hitbox.move(_player.core.velocity);
    _player.render.bodySprite->setPosition(_player.core.hitbox.getPosition());
    UpdateGunTransform(_player.render.bodySprite, _player.render.gunSprite);
    UpdateGunRotation(_player.render.bodySprite, _player.render.gunSprite);

    // Aktualizacja pocisków gracza
    for (auto it = _bullets.begin(); it != _bullets.end();) {
        it->sprite.move(it->velocity);
        it->hitbox.setPosition(it->sprite.getPosition());
        // Sprawdzenie kolizji z przeciwnikiem (hostem)
        if (_enemy.core.hitbox.getGlobalBounds().intersects(it->hitbox.getGlobalBounds())) {
            // Informacja o trafieniu może być wysłana do hosta lub host sam to wykryje
            it = _bullets.erase(it);
        } else if (it->sprite.getPosition().x < 0 || it->sprite.getPosition().x > _windowSize.x ||
                   it->sprite.getPosition().y < 0 || it->sprite.getPosition().y > _windowSize.y) {
            it = _bullets.erase(it);
        } else {
            ++it;
        }
    }

    // Logika odbierania wiadomości została przeniesiona do ReceiveNetworkData()
    // W metodzie Update() możemy przetwarzać dane przygotowane przez wątek sieciowy,
    // np. aktualizować pozycję przeciwnika na podstawie danych chronionych mutexem.
    // Poniższy blok jest przykładem, jak można by to zrobić, ale konkretna implementacja
    // zależy od tego, jak dane są przekazywane z wątku sieciowego.
    // W tym przykładzie zakładamy, że UpdateEnemyPosition i inne funkcje modyfikujące stan
    // są wywoływane bezpośrednio z wątku sieciowego (z odpowiednią synchronizacją).

    // Aktualizacja pocisków przeciwnika (otrzymanych z sieci)
    // Dostęp do _enemyBullets powinien być chroniony mutexem, jeśli jest modyfikowany przez wątek sieciowy
    std::lock_guard<std::mutex> lock(_dataMutex);
    for (auto it = _enemyBullets.begin(); it != _enemyBullets.end();) {
        it->sprite.move(it->velocity);
        it->hitbox.setPosition(it->sprite.getPosition());
        // Sprawdzenie kolizji z graczem (klientem)
        if (_player.core.hitbox.getGlobalBounds().intersects(it->hitbox.getGlobalBounds())) {
            _player.core.health -= 10; // Przykładowe obrażenia
            if (_udpManager) _udpManager->Send("hit_player"); // Informuj hosta o trafieniu
            it = _enemyBullets.erase(it);
        } else if (it->sprite.getPosition().x < 0 || it->sprite.getPosition().x > _windowSize.x ||
                   it->sprite.getPosition().y < 0 || it->sprite.getPosition().y > _windowSize.y) {
            it = _enemyBullets.erase(it);
        } else {
            ++it;
        }
    }

    DisplayPlayerData(_player);
    DisplayPlayerData(_enemy);
}

void GameplayStateGuest::ReceiveNetworkData() {
    while (_running) {
        if (_udpManager && _udpManager->HasMessages()) {
            std::string msg = _udpManager->PopMessage();
            //std::cout << "Guest Received MSG (Thread): " << msg << std::endl; // LOG RAW MESSAGE

            size_t typeEnd = msg.find(';');
            if (typeEnd != std::string::npos) {
                std::string type = msg.substr(0, typeEnd);
                std::string data = msg.substr(typeEnd + 1);
                //std::cout << "Guest Parsed Type (Thread): [" << type << "], Data: [" << data << "]" << std::endl;

                std::lock_guard<std::mutex> lock(_dataMutex); // Lock before accessing shared data
                if (type == "pos_enemy") {
                    size_t sep = data.find(';');
                    if (sep != std::string::npos) {
                        try {
                            float x = std::stof(data.substr(0, sep));
                            float y = std::stof(data.substr(sep + 1));
                            //std::cout << "Guest Applying pos_enemy (Thread): x=" << x << ", y=" << y << std::endl;
                            UpdateEnemyPosition({x, y}); // This function modifies shared data (_enemy)
                        } catch (const std::invalid_argument& ia) {
                            std::cerr << "Guest ERR (Thread): Invalid argument for std::stof in pos_enemy: " << ia.what() << " | Data: " << data << std::endl;
                        } catch (const std::out_of_range& oor) {
                            std::cerr << "Guest ERR (Thread): Out of range for std::stof in pos_enemy: " << oor.what() << " | Data: " << data << std::endl;
                        }
                    } else {
                         std::cerr << "Guest ERR (Thread): Malformed pos_enemy message (no separator for coords): " << data << std::endl;
                    }
                } else if (type == "gun_angle_enemy") {
                    try {
                        float angle = std::stof(data);
                        //std::cout << "Guest Applying gun_angle_enemy (Thread): " << angle << std::endl;
                        _enemy.render.gunSprite->setRotation(angle); // Modifies shared data
                    } catch (const std::invalid_argument& ia) {
                        std::cerr << "Guest ERR (Thread): Invalid argument for std::stof in gun_angle_enemy: " << ia.what() << " | Data: " << data << std::endl;
                    } catch (const std::out_of_range& oor) {
                        std::cerr << "Guest ERR (Thread): Out of range for std::stof in gun_angle_enemy: " << oor.what() << " | Data: " << data << std::endl;
                    }
                } else if (type == "fire_enemy") {
                    //std::cout << "Guest Received fire_enemy (Thread). Data: " << data << std::endl;
                    // TODO: Implement full deserialization for enemy bullets, ensuring thread safety
                    // For example, add to a temporary queue and process in Update() or directly add to _enemyBullets with mutex.
                } else if (type == "health_enemy") {
                    try {
                        _enemy.core.health = std::stoi(data); // Modifies shared data
                        //std::cout << "Guest Applied health_enemy (Thread): " << _enemy.core.health << std::endl;
                    } catch (const std::invalid_argument& ia) {
                        std::cerr << "Guest ERR (Thread): Invalid argument for std::stoi in health_enemy: " << ia.what() << " | Data: " << data << std::endl;
                    } catch (const std::out_of_range& oor) {
                        std::cerr << "Guest ERR (Thread): Out of range for std::stoi in health_enemy: " << oor.what() << " | Data: " << data << std::endl;
                    }
                } else if (type == "health_player") {
                    try {
                        _player.core.health = std::stoi(data); // Modifies shared data
                        //std::cout << "Guest Applied health_player (Thread): " << _player.core.health << std::endl;
                    } catch (const std::invalid_argument& ia) {
                        std::cerr << "Guest ERR (Thread): Invalid argument for std::stoi in health_player: " << ia.what() << " | Data: " << data << std::endl;
                    } catch (const std::out_of_range& oor) {
                        std::cerr << "Guest ERR (Thread): Out of range for std::stoi in health_player: " << oor.what() << " | Data: " << data << std::endl;
                    }
                } else if (type == "round_over") {
                    //std::cout << "Guest Received round_over (Thread). Winner ID: " << data << std::endl;
                    // Handle round over logic, potentially setting a flag to be checked in Update()
                } else if (type == "restart_game") {
                    //std::cout << "Guest Received restart_game (Thread)." << std::endl;
                    RoundInit(); // Modifies shared game state
                } else {
                    //std::cout << "Guest Received unknown message type (Thread): " << type << std::endl;
                }
            } else {
                //std::cerr << "Guest ERR (Thread): Received malformed message (no type separator ';'): " << msg << std::endl;
            }
        } else {
            // Sleep for a short duration to avoid busy-waiting if no messages
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}


void GameplayStateGuest::UpdateEnemyPosition(sf::Vector2f newPosition) {
    // This function is now called from ReceiveNetworkData, which holds the mutex.
    // No need for an additional lock here if _dataMutex is already held by the caller.
    //std::cout << "Guest LOG: UpdateEnemyPosition called with: x=" << newPosition.x << ", y=" << newPosition.y << std::endl;
    _enemy.core.hitbox.setPosition(newPosition);
    _enemy.render.bodySprite->setPosition(newPosition);
    UpdateGunTransform(_enemy.render.bodySprite, _enemy.render.gunSprite);
    // Enemy gun rotation should be updated by a separate message like \"gun_angle_enemy\"
    // UpdateGunRotation(_enemy.render.bodySprite, _enemy.render.gunSprite); // This is for local player's gun based on mouse.
}

void GameplayStateGuest::UpdateGunTransform(sf::Sprite *targetSprite, sf::Sprite *gunSprite) {
    if (targetSprite && gunSprite) {
        gunSprite->setPosition(targetSprite->getPosition());
    }
}

// void GameplayStateGuest::UpdateEnemyBullets(const std::vector<Bullet>& newBullets) {
//     // Aktualizacja stanu pocisków przeciwnika
//     _enemy.bullets = newBullets;
//
//     // Synchronizacja pozycji i rotacji
//     for(auto& bullet : _enemy.bullets) {
//         bullet.sprite.setRotation(atan2(bullet.velocity.y, bullet.velocity.x) * 180 / 3.14159f);
//     }
// }

void GameplayStateGuest::UpdateGunRotation(sf::Sprite* targetSprite, sf::Sprite* gunSprite) {
    sf::Vector2f direction = _mousePosition - targetSprite->getPosition();
    float angle = atan2(direction.y, direction.x) * 180 / PI;
    gunSprite->setRotation(angle);
}

void GameplayStateGuest::DisplayPlayerData(Player &p) {
    sf::Vector2f position = p.render.bodySprite->getPosition();
    sf::FloatRect bounds = p.render.bodySprite->getGlobalBounds();
    sf::FloatRect textBounds = p.render.indicatorText->getGlobalBounds();

    float textX = position.x - (textBounds.width / 2);  // wyśrodkuj horyzontalnie
    float textY = position.y - (bounds.height / 2) - 34;

    p.render.healthBarBackground.setPosition(position.x - 30.f, position.y - bounds.height/2 - 10.f);
    p.render.healthBarFill.setPosition(position.x - 28.f, position.y - bounds.height/2 - 12.f);

    p.render.indicatorText->setPosition(textX, textY);
}

bool GameplayStateGuest::CheckWin() {
    return _player.core.health == 0 || _enemy.core.health == 0;
}



void GameplayStateGuest::FireBullet(sf::Sprite* sourceSprite, sf::Sprite* gunSprite) {
    Bullet newBullet;
    newBullet.sprite.setTexture(_data->assetManager.GetTexture("bulletBlue"));

    // Oblicz pozycję startową
    float angle = gunSprite->getRotation() * PI / 180.f;
    float gunLength = gunSprite->getGlobalBounds().width;
    newBullet.sprite.setPosition(
        gunSprite->getPosition().x + cos(angle) * gunLength,
        gunSprite->getPosition().y + sin(angle) * gunLength
    );

    newBullet.hitbox.setSize(newBullet.sprite.getGlobalBounds().getSize());
    newBullet.hitbox.setPosition(newBullet.sprite.getPosition());
    newBullet.hitbox.setOutlineColor(sf::Color::Green);
    newBullet.hitbox.setFillColor(sf::Color::Transparent);
    newBullet.hitbox.setOutlineThickness(1.0f);

    // Kierunek strzału
    newBullet.velocity = sf::Vector2f(cos(angle), sin(angle)) * newBullet.speed;
    newBullet.sprite.setRotation(angle * 180 / PI);

    _bullets.push_back(newBullet);
}

void GameplayStateGuest::DrawCustomCrosshair() {
    float gap = 4.0f;
    float lineLength = 6.0f;
    float lineThickness = 2.0f;
    float dotRadius = 2.0f;

    _crosshair.dot.setRadius(dotRadius);
    _crosshair.dot.setFillColor(sf::Color::Green);
    _crosshair.dot.setOrigin(dotRadius, dotRadius);
    _crosshair.dot.setPosition(_mousePosition);
    _data->window.draw(_crosshair.dot);

    sf::RectangleShape line;
    line.setFillColor(sf::Color::Green);

    line.setSize(sf::Vector2f(lineThickness, lineLength));
    line.setOrigin(lineThickness / 2, lineLength);
    line.setPosition(_mousePosition.x, _mousePosition.y - gap);
    _data->window.draw(line);

    line.setOrigin(lineThickness / 2, 0);
    line.setPosition(_mousePosition.x, _mousePosition.y + gap);
    _data->window.draw(line);

    line.setSize(sf::Vector2f(lineLength, lineThickness));
    line.setOrigin(lineLength, lineThickness / 2);
    line.setPosition(_mousePosition.x - gap, _mousePosition.y);
    _data->window.draw(line);

    line.setOrigin(0, lineThickness / 2);
    line.setPosition(_mousePosition.x + gap, _mousePosition.y);
    _data->window.draw(line);
}




void GameplayStateGuest::Draw() {
    _data->window.clear();

    _data->window.draw(*_player.render.gunSprite);
    _data->window.draw(*_player.render.bodySprite);
    _data->window.draw(*_player.render.indicatorText);
    _data->window.draw(_player.render.healthBarBackground);
    _data->window.draw(_player.render.healthBarFill);

    _data->window.draw(*_enemy.render.gunSprite);
    _data->window.draw(*_enemy.render.bodySprite);
    _data->window.draw(*_enemy.render.indicatorText);
    _data->window.draw(_enemy.render.healthBarBackground);
    _data->window.draw(_enemy.render.healthBarFill);

    if (_hitboxVisibility) {
        _data->window.draw(_enemy.core.hitbox);
        _data->window.draw(_player.core.hitbox);
    }

    if (CheckWin()) {
        _data->window.draw(*_roundOverText);
        _data->window.draw(*_restartText);
    }

    for (const auto& bullet : _bullets) {
        _data->window.draw(bullet.sprite);
        if (_hitboxVisibility) {
            _data->window.draw(bullet.hitbox);
        }
    }

    DrawCustomCrosshair();

    _data->window.display();
}


GameplayStateGuest::~GameplayStateGuest() {
    _running = false; // Signal the network thread to stop
    if (_networkThread.joinable()) {
        _networkThread.join(); // Wait for the network thread to finish
    }

    delete _player.render.indicatorText;
    delete _enemy.render.indicatorText;

    delete _player.render.bodySprite;
    delete _player.render.gunSprite;

    delete _enemy.render.bodySprite;
    delete _enemy.render.gunSprite;

    _data->window.setMouseCursorVisible(true);
}


void GameplayStateGuest::ClearObjects() {
    _bullets.clear();
}