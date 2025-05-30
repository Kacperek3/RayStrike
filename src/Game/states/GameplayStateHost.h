// src/Game/states/GameplayStateHost.h
#pragma once
#include "Game.h"
#include "State.h"
#include <vector>
#include "UdpNetworkManager.h"

class GameplayStateHost : public State {
public:
    GameplayStateHost(GameDataRef data, int tcpSocketServer);
    ~GameplayStateHost();
    void Init() override;
    void HandleInput() override;
    void Update() override;
    void Draw() override;
    void ClearObjects() override;

private:
    // Re-using the same structures as in GameplayStateGuest
    struct Bullet {
        sf::Sprite sprite;
        sf::Vector2f velocity;
        sf::RectangleShape hitbox;
        float speed = 15.0f;
        int ownerId = 0; // 0 for host, 1 for guest
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
    Player _player; // Host's player
    Player _enemy;  // Guest's player (controlled remotely)
    
    std::vector<Bullet> _bullets;       // Bullets fired by the host OR guest, ownerId will differentiate

    sf::Vector2u _windowSize;
    Crosshair _crosshair;
    UdpNetworkManager* _udpManager; // UDP Network Manager for host

    sf::Text *_roundOverText;
    sf::Text *_restartText;
    bool _roundOver = false;
    bool _guestRestartRequested = false;


    int _tcpSocketServer; // TCP socket for server communication

    sf::Font _font;
    sf::Vector2f _mousePosition;

    bool _hitboxVisibility = false;

    // Initialization and Update helpers
    void InitPlayer(Player &p, float x, float y, bool isHost, const std::string& name, const std::string& playerTexture, const std::string& gunTexture);
    void RoundInit();
    void UpdateGunTransform(sf::Sprite *targetSprite, sf::Sprite *gunSprite);
    void UpdateGunRotation(sf::Sprite* targetSprite, sf::Sprite* gunSprite, const sf::Vector2f& mousePos);
    void DisplayPlayerData(Player &p);
    void DrawCustomCrosshair();

    void FireBullet(Player& shooter, sf::Sprite* gunSprite, float angleDegrees, bool isHostBullet);
    
    // Network message handling
    void ProcessGuestMessage(const std::string& msg);
    void SendGameStateToGuest(); // Sends relevant parts of the game state

    bool CheckWin(); // Returns true if a player's health is 0
    void HandleCollisions();
};