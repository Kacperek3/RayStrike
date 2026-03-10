#pragma once
#include "../network/UdpNetworkManager.h"
#include "Game.h"
#include "State.h"
#include <atomic> // Added for std::atomic
#include <mutex>  // Added for std::mutex
#include <thread> // Added for std::thread
#include <vector>

class GameplayStateGuest : public State {
  public:
    GameplayStateGuest(GameDataRef data, int tcpSocketClient, std::string hostName,
                       std::string guestName);
    ~GameplayStateGuest();
    void Init() override;
    void HandleInput() override;
    void Update() override;
    void Draw() override;
    void ClearObjects() override;

  private:
    std::string _hostName;
    std::string _guestName;
    struct Bullet {
        sf::Sprite sprite;
        sf::Vector2f velocity;
        sf::RectangleShape hitbox;
        float speed = 15.0f;
    };
    struct PlayerCore {
        sf::CircleShape hitbox;
        sf::Vector2f velocity;
        float speed = 5.0f;
        int health = 100;
    };
    struct PlayerRender {
        sf::Sprite *bodySprite;
        sf::Sprite *gunSprite;
        sf::Text *indicatorText;
        sf::RectangleShape healthBarBackground;
        sf::RectangleShape healthBarFill;
    };
    struct Player {
        PlayerCore core;
        PlayerRender render;
    };
    struct Crosshair {
        sf::CircleShape dot;
        sf::RectangleShape line;
    };

    GameDataRef _data;
    Player _player;
    Player _enemy;
    std::vector<Bullet> _bullets;
    std::vector<Bullet> _enemyBullets;
    sf::Vector2u _windowSize;

    Crosshair _crosshair;
    UdpNetworkManager *_udpManager;

    // Network thread members
    std::thread _networkThread;
    std::mutex _dataMutex;
    std::atomic<bool> _running;

    sf::Text *_roundOverText;
    sf::Text *_restartText;

    sf::Font _font;
    sf::Vector2f _mousePosition;

    bool _hitboxVisibility = false;
    int _tcpSocketClient;

    std::vector<sf::RectangleShape> _walls;

    int _hostScore = 0;
    int _guestScore = 0;
    sf::Text _scoreText;

    void UpdateEnemyPosition(sf::Vector2f newPosition);
    void UpdateEnemyBullets(
        const std::vector<Bullet> &newBullets); // This might need adjustment or removal if bullets
                                                // are handled differently with threading

    // Network thread function
    void ReceiveNetworkData();

    void RoundInit();
    void UpdateGunTransform(sf::Sprite *targetSprite, sf::Sprite *gunSprite);

    void FireBullet(sf::Sprite *sourceSprite, sf::Sprite *gunSprite);

    void UpdateGunRotation(sf::Sprite *targetSprite, sf::Sprite *gunSprite);
    void DisplayPlayerData(Player &p);
    void DrawCustomCrosshair();

    void InitPlayer(Player &p, float x, float y);

    bool CheckWin();
};
