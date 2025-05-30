#pragma once
#include "Game.h"
#include "State.h"
#include <vector>
#include "../network/UdpNetworkManager.h"
#include <thread> // Added for std::thread
#include <mutex>  // Added for std::mutex
#include <atomic> // Added for std::atomic

class GameplayStateGuest : public State {
public:
    GameplayStateGuest(GameDataRef data, int tcpSocketClient = -1);
    ~GameplayStateGuest();
    void Init() override;
    void HandleInput() override;
    void Update() override;
    void Draw() override;
    void ClearObjects() override;

private:
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
    UdpNetworkManager* _udpManager;

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

    void UpdateEnemyPosition(sf::Vector2f newPosition);
    void UpdateEnemyBullets(const std::vector<Bullet>& newBullets); // This might need adjustment or removal if bullets are handled differently with threading

    // Network thread function
    void ReceiveNetworkData();

    void RoundInit();
    void UpdateGunTransform(sf::Sprite *targetSprite, sf::Sprite *gunSprite);

    void FireBullet(sf::Sprite* sourceSprite, sf::Sprite* gunSprite);

    void UpdateGunRotation(sf::Sprite* targetSprite, sf::Sprite* gunSprite);
    void DisplayPlayerData(Player &p);
    void DrawCustomCrosshair();

    void InitPlayer(Player &p, float x, float y);

    bool CheckWin();
};