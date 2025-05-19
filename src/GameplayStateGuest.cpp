#include "GameplayStateGuest.h"

#include <cmath>

#define PI 3.14159f

GameplayStateGuest::GameplayStateGuest(GameDataRef data) : _data(data) {
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
            _data->window.close();
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
        }
    }

    _player.core.velocity = {0, 0};

    if (CheckWin()) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
            RoundInit();
        }
        return;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) _player.core.velocity.y = -_player.core.speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) _player.core.velocity.y = _player.core.speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) _player.core.velocity.x = -_player.core.speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) _player.core.velocity.x = _player.core.speed;
}

void GameplayStateGuest::UpdateEnemyPosition(sf::Vector2f newPosition) {
    _enemy.render.bodySprite->setPosition(newPosition);
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


void GameplayStateGuest::Update() {


    _enemy.core.hitbox.setPosition(_enemy.render.bodySprite->getPosition());

    _player.core.hitbox.move(_player.core.velocity);
    _player.render.bodySprite->setPosition(_player.core.hitbox.getPosition());


    _player.render.gunSprite->setPosition(_player.render.bodySprite->getPosition());
    UpdateGunRotation(_player.render.bodySprite, _player.render.gunSprite);

    // Don't update gun position for enemy.
    _enemy.render.gunSprite->setPosition(_enemy.render.bodySprite->getPosition());

    float healthPercentage = static_cast<float>(_player.core.health) / 100.f;
    _player.render.healthBarFill.setSize(sf::Vector2f(56.f * healthPercentage, 6.f));

    healthPercentage = static_cast<float>(_enemy.core.health) / 100.f;
    _enemy.render.healthBarFill.setSize(sf::Vector2f(56.f * healthPercentage, 6.f));

    DisplayPlayerData(_player);
    DisplayPlayerData(_enemy);

    sf::Vector2f position = _player.render.bodySprite->getPosition();
    position.x = std::clamp(position.x, 0.f, static_cast<float>(_windowSize.x));
    position.y = std::clamp(position.y, 0.f, static_cast<float>(_windowSize.y));
    _player.render.bodySprite->setPosition(position);

    for (auto it = _bullets.begin(); it != _bullets.end();) {
        it->sprite.move(it->velocity);
        it->hitbox.setPosition(it->sprite.getPosition());
        bool bulletHit = false;

        sf::Vector2f bulletPos = it->sprite.getPosition();
        sf::Vector2f enemyCenter = _enemy.core.hitbox.getPosition();
        float distance = std::hypot(bulletPos.x - enemyCenter.x,
                                  bulletPos.y - enemyCenter.y);
        float sumRadius = _enemy.core.hitbox.getRadius() + it->sprite.getGlobalBounds().width/2;
        if (distance <= sumRadius) {
            _enemy.core.health = std::max(0, _enemy.core.health - 10);
            bulletHit = true;
        }


        // Kolizja z granicami ekranu
        sf::FloatRect bulletBounds = it->sprite.getGlobalBounds();
        if (bulletBounds.left < -100 || bulletBounds.left > _windowSize.x + 100 ||
            bulletBounds.top < -100 || bulletBounds.top > _windowSize.y + 100) {
            bulletHit = true;
            }

        if (bulletHit) {
            it = _bullets.erase(it);
        } else {
            ++it;
        }
    }
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