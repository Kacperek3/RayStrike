// src/Game/states/GameplayStateHost.cpp
#include "GameplayStateHost.h"
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

#define PI 3.14159f

GameplayStateHost::GameplayStateHost(GameDataRef data, int tcpSocketServer, std::string hostName,
                                     std::string guestName)
    : _data(data), _tcpSocketServer(tcpSocketServer), _networkThreadRunning(false),
      _hostName(hostName), _guestName(guestName) {
    _windowSize = _data->window.getSize();
    _data->window.setMouseCursorVisible(false);

    _player.render.indicatorText = new sf::Text();
    _enemy.render.indicatorText = new sf::Text();
    _roundOverText = new sf::Text();
    _restartText = new sf::Text();
    _player.render.bodySprite = new sf::Sprite();
    _player.render.gunSprite = new sf::Sprite();
    _enemy.render.bodySprite = new sf::Sprite();
    _enemy.render.gunSprite = new sf::Sprite();

    // Initialize UdpNetworkManager for Host
    _udpManager = new UdpNetworkManager(_tcpSocketServer, -1);
    if (!_udpManager->Initialize()) {
        std::cerr << "Host: UdpNetworkManager initialization failed!" << std::endl;
        // Handle initialization failure, maybe throw an exception or set an error state
    } else {
        // Start the network thread only if UDP manager initialized successfully
        _networkThreadRunning = true;
        _networkThread = std::thread(&GameplayStateHost::NetworkThreadFunc, this);
    }
}

GameplayStateHost::~GameplayStateHost() {
    _networkThreadRunning = false;
    if (_networkThread.joinable()) {
        _networkThread.join();
    }

    delete _player.render.indicatorText;
    delete _enemy.render.indicatorText;
    delete _roundOverText;
    delete _restartText;
    delete _player.render.bodySprite;
    delete _player.render.gunSprite;
    delete _enemy.render.bodySprite;
    delete _enemy.render.gunSprite;
    delete _udpManager;
    _data->window.setMouseCursorVisible(true);
}

void GameplayStateHost::InitPlayer(Player &p, float x, float y, bool isHost,
                                   const std::string &name, const std::string &playerTextureKey,
                                   const std::string &gunTextureKey) {
    p.render.bodySprite->setTexture(_data->assetManager.GetTexture(playerTextureKey));
    p.render.gunSprite->setTexture(_data->assetManager.GetTexture(gunTextureKey));

    p.render.bodySprite->setOrigin(p.render.bodySprite->getLocalBounds().width / 2,
                                   p.render.bodySprite->getLocalBounds().height / 2);
    p.render.bodySprite->setPosition(x, y);
    p.render.bodySprite->setScale(2.0f, 2.0f);

    sf::FloatRect playerBounds = p.render.bodySprite->getGlobalBounds();
    p.core.hitbox.setRadius(playerBounds.width / 2);
    p.core.hitbox.setOrigin(playerBounds.width / 2, playerBounds.height / 2);
    p.core.hitbox.setPosition(p.render.bodySprite->getPosition());
    p.core.hitbox.setFillColor(sf::Color::Transparent);
    p.core.hitbox.setOutlineColor(isHost ? sf::Color::Blue
                                         : sf::Color::Red); // Different colors for host/enemy
    p.core.hitbox.setOutlineThickness(2.0f);

    p.render.gunSprite->setOrigin(5.f, p.render.gunSprite->getLocalBounds().height /
                                           2); // Adjust origin as needed
    p.render.gunSprite->setPosition(p.render.bodySprite->getPosition());

    p.render.indicatorText->setFont(_font);
    p.render.indicatorText->setString(name);
    p.render.indicatorText->setCharacterSize(14);
    p.render.indicatorText->setFillColor(sf::Color::White);
    // Position will be updated in DisplayPlayerData

    p.render.healthBarBackground.setSize(sf::Vector2f(60.f, 8.f));
    p.render.healthBarBackground.setFillColor(sf::Color(50, 50, 50));
    p.render.healthBarFill.setSize(sf::Vector2f(56.f, 6.f));     // Full width initially
    p.render.healthBarFill.setFillColor(sf::Color(30, 200, 30)); // Green for health
}

void GameplayStateHost::RoundInit() {
    std::lock_guard<std::mutex> lock(_gameStateMutex); // Lock before modifying shared data
    _player.core.health = 100;
    _enemy.core.health = 100;
    _bullets.clear();
    _roundOver = false;
    _guestRestartRequested = false;

    // Initial positions
    InitPlayer(_player, _windowSize.x - 100, _windowSize.y / 2, true, "Host", "player", "pgun");
    InitPlayer(_enemy, 100, _windowSize.y / 2, false, "Guest", "enemy", "egun");

    // Initial state send can be triggered by the network thread or here with a lock
    // SendGameStateToGuest(); // Consider if this should be immediate or handled by network thread
}

void GameplayStateHost::Init() {
    _data->assetManager.LoadTexture("pgun", "assets/pgun2.png");
    _data->assetManager.LoadTexture("egun", "assets/egun2.png");
    _data->assetManager.LoadTexture("player", "assets/player1.png");
    _data->assetManager.LoadTexture("enemy", "assets/enemy1.png");
    _data->assetManager.LoadTexture("bulletBlue", "assets/bulletBlue.png");
    _data->assetManager.LoadTexture("bulletRed", "assets/bulletRed.png");

    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cerr << "Failed to load font in GameplayStateHost" << std::endl;
    }

    _roundOverText->setFont(_font);
    _roundOverText->setString("Round Over");
    _roundOverText->setCharacterSize(44);
    _roundOverText->setFillColor(sf::Color::White);
    // Position will be set when drawn

    _restartText->setFont(_font);
    _restartText->setString("Press R to restart"); // Host can restart
    _restartText->setCharacterSize(44);
    _restartText->setFillColor(sf::Color::White);
    // Position will be set when drawn

    _crosshair.dot.setFillColor(sf::Color::Red);
    _crosshair.dot.setRadius(1.0f);
    _crosshair.line.setFillColor(sf::Color::Green);
    _crosshair.line.setSize(sf::Vector2f(5.0f, 10.0f));

    auto createNeonWall = [](float width, float height, float posX, float posY) {
        sf::RectangleShape wall(sf::Vector2f(width, height));
        wall.setPosition(posX, posY);
        wall.setFillColor(sf::Color(30, 30, 30, 220));
        wall.setOutlineThickness(3.f);
        wall.setOutlineColor(sf::Color(0, 200, 255));
        return wall;
    };

    _walls.push_back(
        createNeonWall(50.f, 300.f, _windowSize.x / 2 - 25.f, _windowSize.y / 2 - 150.f));

    float guestWallX = 250.f;
    _walls.push_back(createNeonWall(40.f, 120.f, guestWallX, _windowSize.y / 4 - 60.f)); // Górna
    _walls.push_back(
        createNeonWall(40.f, 120.f, guestWallX, _windowSize.y * 3 / 4 - 60.f)); // Dolna

    float hostWallX = _windowSize.x - 290.f; // 250 + 40 szerokości ściany = 290
    _walls.push_back(createNeonWall(40.f, 120.f, hostWallX, _windowSize.y / 4 - 60.f));     // Górna
    _walls.push_back(createNeonWall(40.f, 120.f, hostWallX, _windowSize.y * 3 / 4 - 60.f)); // Dolna

    _scoreText.setFont(_font);
    _scoreText.setCharacterSize(28);
    _scoreText.setFillColor(sf::Color(245, 245, 245));
    _scoreText.setOutlineThickness(1.5f);
    _scoreText.setOutlineColor(sf::Color(15, 15, 15));
    _scoreText.setLetterSpacing(1.2f);

    _scoreText.setString(_hostName + "   0  -  0   " + _guestName);

    sf::FloatRect scoreBounds = _scoreText.getLocalBounds();
    _scoreText.setPosition(_windowSize.x / 2 - scoreBounds.width / 2, 15.f);

    InitPlayer(_player, _windowSize.x - 100, _windowSize.y / 2, true, _hostName, "player", "pgun");
    InitPlayer(_enemy, 100, _windowSize.y / 2, false, _guestName, "enemy", "egun");
}

void GameplayStateHost::HandleInput() {
    sf::Event event;
    while (_data->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _data->window.close();
            if (_udpManager)
                _udpManager->Send("disconnect_host"); // Inform guest
        }
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::H) {
                _hitboxVisibility = !_hitboxVisibility;
            }
            if (_roundOver && event.key.code == sf::Keyboard::R) {
                RoundInit(); // Host initiates restart
                if (_udpManager)
                    _udpManager->Send("restart_game");
            }
        }
        if (_roundOver)
            continue; // No game input if round is over

        if (event.type == sf::Event::MouseMoved) {
            _mousePosition = _data->inputManager.GetMousePosition(_data->window);
        }
        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left) {
            float angle = _player.render.gunSprite->getRotation();
            FireBullet(_player, _player.render.gunSprite, angle, true);
        }
    }

    if (_roundOver)
        return;

    _player.core.velocity = {0, 0};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        _player.core.velocity.y = -_player.core.speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        _player.core.velocity.y = _player.core.speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        _player.core.velocity.x = -_player.core.speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        _player.core.velocity.x = _player.core.speed;
}

void GameplayStateHost::ProcessGuestMessage(const std::string &msg) {
    // This function will be called from the network thread, so lock access to shared game state
    std::lock_guard<std::mutex> lock(_gameStateMutex);

    size_t typeEnd = msg.find(';');
    if (typeEnd == std::string::npos)
        return; // Invalid message format

    std::string type = msg.substr(0, typeEnd);
    std::string data = msg.substr(typeEnd + 1);

    if (type == "pos") {
        size_t sep = data.find(';');
        if (sep != std::string::npos) {
            try {
                float x = std::stof(data.substr(0, sep));
                float y = std::stof(data.substr(sep + 1));
                _enemy.core.hitbox.setPosition(x, y); // Update guest's (enemy) position
                _enemy.render.bodySprite->setPosition(x, y);
            } catch (const std::invalid_argument &ia) {
                std::cerr << "Invalid argument for pos: " << ia.what() << std::endl;
            }
        }
    } else if (type == "fire") {
        try {

            size_t semicolonPos = data.find(';');
            std::string angleStr = data.substr(semicolonPos + 1);

            size_t newlinePos = angleStr.find_first_of("\r\n");
            if (newlinePos != std::string::npos)
                angleStr = angleStr.substr(0, newlinePos);

            float angle = std::stof(angleStr);
            std::cout << "HOST received fire angle: " << angle << std::endl;

            FireBullet(_enemy, _enemy.render.gunSprite, angle, false);
        } catch (const std::invalid_argument &ia) {
            std::cerr << "Invalid argument for fire angle: " << ia.what() << std::endl;
        } catch (const std::out_of_range &oor) { // DODANO OBSŁUGĘ std::out_of_range
            std::cerr << "Out of range for fire angle: " << oor.what() << std::endl;
        }
    } else if (type == "hit_player") { // Guest reports it hit the host
        _player.core.health = std::max(0, _player.core.health - 10); // Example damage
    } else if (type == "gun_angle_enemy") {
        try {
            float angle = std::stof(data);
            _enemy.render.gunSprite->setRotation(angle); // Update guest's gun rotation
        } catch (const std::invalid_argument &ia) {
            std::cerr << "Invalid argument for gun_angle_enemy: " << ia.what() << std::endl;
        } catch (const std::out_of_range &oor) {
            std::cerr << "Out of range for gun_angle_enemy: " << oor.what() << std::endl;
        }
    } else if (type == "pos_enemy") {
        size_t sep = data.find(';');
        if (sep != std::string::npos) {
            try {
                float x = std::stof(data.substr(0, sep));
                float y = std::stof(data.substr(sep + 1));
                _player.core.hitbox.setPosition(x, y); // Update host's position
                _player.render.bodySprite->setPosition(x, y);
            } catch (const std::invalid_argument &ia) {
                std::cerr << "Invalid argument for pos_enemy: " << ia.what() << std::endl;
            }
        }

    } else if (type == "disconnect") {
        std::cout << "Guest disconnected." << std::endl;
        // Handle guest disconnection, e.g., end game or wait for new guest
        _roundOver = true; // Or some other state
    } else if (type == "restart_request") {
        _guestRestartRequested = true;
        // If host also wants to restart (or auto-restarts), then proceed.
        if (_roundOver &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::R)) { // Check if host also pressed R
            RoundInit();
            if (_udpManager)
                _udpManager->Send("restart_game");
        }
    }
    // Add more message types as needed (e.g., guest gun rotation)
}

void GameplayStateHost::SendGameStateToGuest() {
    // This function will be called from the network thread, so lock access to shared game state
    std::lock_guard<std::mutex> lock(_gameStateMutex);

    if (!_udpManager || !_udpManager->IsConnected())
        return;

    std::ostringstream oss;
    // Host Player (becomes "enemy" for the guest)
    oss << "pos_enemy;" << std::fixed << std::setprecision(2) << _player.core.hitbox.getPosition().x
        << ";" << _player.core.hitbox.getPosition().y << "\n";
    _udpManager->Send(oss.str()); // Send position immediately
    oss.str("");                  // Clear the string stream for next message

    oss << "health_enemy;" << _player.core.health << "\n";
    _udpManager->Send(oss.str()); // Send position immediately
    oss.str("");                  // Clear the string stream for next message

    oss << "gun_angle_enemy;" << _player.render.gunSprite->getRotation() << "\n";
    _udpManager->Send(oss.str()); // Send position immediately
    oss.str("");                  // Clear the string stream for next message

    // Guest Player (is "player" for the guest)
    oss << "pos_player;" << std::fixed << std::setprecision(2) << _enemy.core.hitbox.getPosition().x
        << ";" << _enemy.core.hitbox.getPosition().y << "\n";
    _udpManager->Send(oss.str()); // Send position immediately
    oss.str("");                  // Clear the string stream for next message

    oss << "health_player;" << _enemy.core.health << "\n";
    _udpManager->Send(oss.str()); // Send position immediately
    oss.str("");                  // Clear the string stream for next message

    oss << "gun_angle_player;" << _enemy.render.gunSprite->getRotation() << "\n";
    _udpManager->Send(oss.str()); // Send position immediately
    oss.str("");                  // Clear the string stream for next message

    // Bullets - send all bullets with their owner and state
    for (const auto &bullet : _bullets) {
        // Format: bullet;x;y;velX;velY;rotation;owner
        oss << "bullet;" << bullet.sprite.getPosition().x << ";" << bullet.sprite.getPosition().y
            << ";" << bullet.velocity.x << ";" << bullet.velocity.y << ";"
            << bullet.sprite.getRotation() << ";" << bullet.ownerId << "\n";
        _udpManager->Send(oss.str()); // Send position immediately
        oss.str("");                  // Clear the string stream for next message
    }

    if (_roundOver) {
        oss << "score;" << _hostScore << ";" << _guestScore << "\n";
        _udpManager->Send(oss.str());
        oss.str("");
    }

    //_udpManager->Send(oss.str());
}

void GameplayStateHost::Update() {
    // Game logic updates remain in the main thread.
    // Network operations (send/receive) are handled by _networkThread.

    if (_roundOver) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
            if (_guestRestartRequested) {
                RoundInit(); // Host and guest both ready
            } else {
                // Host wants to restart, could send a message if guest needs to confirm
                // For now, assume host can unilaterally decide to restart if guest also pressed R
                // (which sets _guestRestartRequested) Or, if we want host to initiate, send a
                // "restart_request" and wait for guest "restart_ack"
            }
        }
        // Display round over messages, etc.
        return; // Don't process game logic if round is over
    }

    // --- Update Host Player (Main Thread) ---
    {
        std::lock_guard<std::mutex> lock(_gameStateMutex); // Lock for reading/writing player data

        // Zapisz pozycję przed ruchem
        sf::Vector2f oldPos = _player.core.hitbox.getPosition();

        // Wykonaj ruch
        _player.core.hitbox.move(_player.core.velocity);

        // Zabezpieczenie na ściany (jeśli wejdzie w ścianę, cofnij ruch)
        for (const auto &wall : _walls) {
            if (_player.core.hitbox.getGlobalBounds().intersects(wall.getGlobalBounds())) {
                _player.core.hitbox.setPosition(oldPos);
                break;
            }
        }

        // Clamp player position to screen bounds (po sprawdzeniu ścian)
        sf::Vector2f pos = _player.core.hitbox.getPosition();
        pos.x = std::clamp(pos.x, _player.core.hitbox.getRadius(),
                           _windowSize.x - _player.core.hitbox.getRadius());
        pos.y = std::clamp(pos.y, _player.core.hitbox.getRadius(),
                           _windowSize.y - _player.core.hitbox.getRadius());
        _player.core.hitbox.setPosition(pos);

        _player.render.bodySprite->setPosition(_player.core.hitbox.getPosition());
        UpdateGunTransform(_player.render.bodySprite, _player.render.gunSprite);
        UpdateGunRotation(_player.render.bodySprite, _player.render.gunSprite, _mousePosition);
    }

    {
        std::lock_guard<std::mutex> lock(_gameStateMutex); // Lock for reading enemy data
        _enemy.render.bodySprite->setPosition(_enemy.core.hitbox.getPosition());
        UpdateGunTransform(_enemy.render.bodySprite, _enemy.render.gunSprite);
        // Gun rotation for enemy is updated by ProcessGuestMessage
    }

    {
        std::lock_guard<std::mutex> lock(_gameStateMutex); // Lock for accessing bullets vector
        for (auto it = _bullets.begin(); it != _bullets.end();) {
            it->sprite.move(it->velocity);
            it->hitbox.setPosition(it->sprite.getPosition());

            // Remove bullets that go off-screen
            if (it->sprite.getPosition().x < 0 || it->sprite.getPosition().x > _windowSize.x ||
                it->sprite.getPosition().y < 0 || it->sprite.getPosition().y > _windowSize.y) {
                it = _bullets.erase(it);
            } else {
                ++it;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(_gameStateMutex);
        HandleCollisions();

        if (!_roundOver && CheckWin()) {
            _roundOver = true;

            // --- Logika przyznawania punktów wyników ---
            if (_player.core.health <= 0) {
                _guestScore++;
            } else if (_enemy.core.health <= 0) {
                _hostScore++;
            }

            _scoreText.setString(_hostName + "   " + std::to_string(_hostScore) + "  -  " +
                                 std::to_string(_guestScore) + "   " + _guestName);
        }
    }
    {
        std::lock_guard<std::mutex> lock(_gameStateMutex);
        DisplayPlayerData(_player);
        DisplayPlayerData(_enemy);
    }
}

void GameplayStateHost::NetworkThreadFunc() {
    std::cout << "Host: NetworkThreadFunc started." << std::endl;
    while (_networkThreadRunning) {
        if (_udpManager && _udpManager->IsConnected()) {
            // Receive messages from Guest
            while (_udpManager->HasMessages()) {
                std::string msg = _udpManager->PopMessage();
                if (!msg.empty()) {
                    ProcessGuestMessage(msg);
                }
            }

            SendGameStateToGuest();

            _scoreText.setString(_hostName + "   " + std::to_string(_hostScore) + "  -  " +
                                 std::to_string(_guestScore) + "   " + _guestName);
        } else {
            if (!_udpManager) {
                std::cerr << "Host: NetworkThreadFunc: _udpManager is null!" << std::endl;
            } else if (!_udpManager->IsConnected()) {
                // std::cerr << "Host: NetworkThreadFunc: UDP manager not connected." << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // Approx 60 updates/sec
    }
    std::cout << "Host: NetworkThreadFunc ended." << std::endl;
}

void GameplayStateHost::HandleCollisions() {
    // Assumes _gameStateMutex is already locked by the caller (Update method)
    for (auto it = _bullets.begin(); it != _bullets.end();) {
        bool hit = false;
        for (const auto &wall : _walls) {
            if (it->hitbox.getGlobalBounds().intersects(wall.getGlobalBounds())) {
                hit = true;
                break;
            }
        }
        // Collision: Host's bullets with Guest
        if (it->ownerId == 0 &&
            _enemy.core.hitbox.getGlobalBounds().intersects(it->hitbox.getGlobalBounds())) {
            _enemy.core.health = std::max(0, _enemy.core.health - 10);
            hit = true;
        }
        // Collision: Guest's bullets with Host
        else if (it->ownerId == 1 &&
                 _player.core.hitbox.getGlobalBounds().intersects(it->hitbox.getGlobalBounds())) {
            _player.core.health = std::max(0, _player.core.health - 10);
            hit = true;
        }

        if (hit) {
            it = _bullets.erase(it);
        } else {
            ++it;
        }
    }
}

void GameplayStateHost::FireBullet(Player &shooter, sf::Sprite *gunSprite, float angleDegrees,
                                   bool isHostBullet) {
    Bullet newBullet;
    newBullet.sprite.setTexture(
        _data->assetManager.GetTexture(isHostBullet ? "bulletBlue" : "bulletRed"));
    newBullet.ownerId = isHostBullet ? 0 : 1;

    float angleRad = angleDegrees * PI / 180.f;
    float gunLength =
        gunSprite->getGlobalBounds().width * gunSprite->getScale().x; // Consider scale

    sf::Vector2f gunTipOffset(cos(angleRad) * gunLength, sin(angleRad) * gunLength);
    sf::Vector2f gunPivot = gunSprite->getPosition(); // This should be the body center

    newBullet.sprite.setPosition(gunPivot + gunTipOffset);
    newBullet.sprite.setRotation(angleDegrees);
    newBullet.sprite.setOrigin(newBullet.sprite.getLocalBounds().width / 2,
                               newBullet.sprite.getLocalBounds().height / 2);

    newBullet.hitbox.setSize(
        sf::Vector2f(newBullet.sprite.getGlobalBounds().width * 0.8f,
                     newBullet.sprite.getGlobalBounds().height * 0.8f)); // Smaller hitbox
    newBullet.hitbox.setOrigin(newBullet.hitbox.getSize().x / 2, newBullet.hitbox.getSize().y / 2);
    newBullet.hitbox.setPosition(newBullet.sprite.getPosition());
    newBullet.hitbox.setFillColor(sf::Color::Transparent);
    newBullet.hitbox.setOutlineColor(sf::Color::Yellow);
    newBullet.hitbox.setOutlineThickness(1.0f);

    newBullet.velocity = sf::Vector2f(cos(angleRad), sin(angleRad)) * newBullet.speed;
    _bullets.push_back(newBullet);
}

void GameplayStateHost::UpdateGunTransform(sf::Sprite *targetSprite, sf::Sprite *gunSprite) {
    gunSprite->setPosition(targetSprite->getPosition());
}

void GameplayStateHost::UpdateGunRotation(sf::Sprite *targetSprite, sf::Sprite *gunSprite,
                                          const sf::Vector2f &mousePos) {
    sf::Vector2f direction = mousePos - targetSprite->getPosition();
    float angle = atan2(direction.y, direction.x) * 180 / PI;
    gunSprite->setRotation(angle);
}

void GameplayStateHost::DisplayPlayerData(Player &p) {
    sf::Vector2f position = p.render.bodySprite->getPosition();
    sf::FloatRect bounds = p.render.bodySprite->getGlobalBounds();

    // Indicator Text (Name)
    sf::FloatRect textBounds = p.render.indicatorText->getGlobalBounds();
    float textX = position.x - (textBounds.width / 2);
    float textY = position.y - (bounds.height / 2) - 34; // Above player
    p.render.indicatorText->setPosition(textX, textY);

    // Health Bar
    p.render.healthBarBackground.setPosition(position.x - 30.f, position.y - (bounds.height / 2) -
                                                                    20.f); // Slightly adjust Y
    p.render.healthBarFill.setPosition(p.render.healthBarBackground.getPosition().x + 2.f,
                                       p.render.healthBarBackground.getPosition().y +
                                           1.f); // Relative to background
    float healthPercentage = static_cast<float>(p.core.health) / 100.f;
    p.render.healthBarFill.setSize(sf::Vector2f(56.f * healthPercentage, 6.f));

    if (&p == &_player) { // Compare addresses to identify the local player
        p.render.healthBarFill.setFillColor(sf::Color::Green);
    } else {
        p.render.healthBarFill.setFillColor(sf::Color::Red);
    }
}

bool GameplayStateHost::CheckWin() {
    // Assumes _gameStateMutex is already locked by the caller (Update method)
    return _player.core.health <= 0 || _enemy.core.health <= 0;
}

void GameplayStateHost::DrawCustomCrosshair() {
    // Same as GameplayStateGuest or customize
    float gap = 4.0f;
    float lineLength = 6.0f;
    float lineThickness = 2.0f;
    float dotRadius = 2.0f;

    _crosshair.dot.setRadius(dotRadius);
    _crosshair.dot.setFillColor(sf::Color::Green); // Host crosshair color
    _crosshair.dot.setOrigin(dotRadius, dotRadius);
    _crosshair.dot.setPosition(_mousePosition); // Use host's mouse position
    _data->window.draw(_crosshair.dot);

    sf::RectangleShape line;
    line.setFillColor(sf::Color::Green);

    line.setSize(sf::Vector2f(lineThickness, lineLength));
    line.setOrigin(lineThickness / 2, lineLength);
    line.setPosition(_mousePosition.x, _mousePosition.y - gap);
    _data->window.draw(line);
    // ... (rest of the crosshair drawing logic)
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

void GameplayStateHost::Draw() {
    _data->window.clear();
    std::lock_guard<std::mutex> lock(_gameStateMutex); // Lock for drawing shared game state
    for (const auto &wall : _walls) {
        _data->window.draw(wall);
    }
    _data->window.draw(_scoreText);

    _data->window.draw(*_player.render.gunSprite);
    _data->window.draw(*_player.render.bodySprite);
    _data->window.draw(*_player.render.indicatorText);
    _data->window.draw(_player.render.healthBarBackground);
    _data->window.draw(_player.render.healthBarFill);
    if (_hitboxVisibility)
        _data->window.draw(_player.core.hitbox);

    // Draw Enemy (Guest)
    _data->window.draw(*_enemy.render.gunSprite);
    _data->window.draw(*_enemy.render.bodySprite);
    _data->window.draw(*_enemy.render.indicatorText);
    _data->window.draw(_enemy.render.healthBarBackground);
    _data->window.draw(_enemy.render.healthBarFill);
    if (_hitboxVisibility)
        _data->window.draw(_enemy.core.hitbox);

    // Draw Bullets
    for (const auto &bullet : _bullets) {
        _data->window.draw(bullet.sprite);
        if (_hitboxVisibility)
            _data->window.draw(bullet.hitbox);
    }

    // Draw Round Over Text
    if (_roundOver) {
        std::string winnerTextStr;
        if (_player.core.health <= 0 && _enemy.core.health <= 0)
            winnerTextStr = "Draw!";
        else if (_player.core.health <= 0)
            winnerTextStr = "Guest Wins!";
        else if (_enemy.core.health <= 0)
            winnerTextStr = "Host Wins!";

        _roundOverText->setString(winnerTextStr);
        sf::FloatRect textBounds = _roundOverText->getGlobalBounds();
        _roundOverText->setPosition(_windowSize.x / 2 - textBounds.width / 2,
                                    _windowSize.y / 3 - textBounds.height / 2);
        _data->window.draw(*_roundOverText);

        textBounds = _restartText->getGlobalBounds();
        _restartText->setPosition(_windowSize.x / 2 - textBounds.width / 2,
                                  _roundOverText->getPosition().y + textBounds.height + 20);
        _data->window.draw(*_restartText);
    }

    DrawCustomCrosshair();
    _data->window.display();
}

void GameplayStateHost::ClearObjects() {
    _bullets.clear();
}
