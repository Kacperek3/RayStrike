// GameEngine.h
#pragma once
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include "UdpNetworkManager.h"
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>

struct PlayerState {
    sf::Vector2f position;
    float rotation;
    int health;
    sf::Vector2f velocity;
    float speed = 5.0f;
};

struct BulletState {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime = 2.0f;
};

struct GameState {
    PlayerState hostPlayer;
    PlayerState clientPlayer;
    std::vector<BulletState> bullets;
};

class GameEngine {
public:
    enum class Role { Host, Client };
    
    GameEngine(Role role, UdpNetworkManager& network);
    
    void Update(float deltaTime);
    void HandleInput(const sf::Event& event);
    void ReceiveNetworkMessage(const std::string& message);

    void AddHostBullet(const BulletState& bullet);
    void AddClientBullet(const BulletState& bullet);
    
    const GameState& GetCurrentState() const { return currentState; }
    
private:
    void HostUpdate(float deltaTime);
    void ClientUpdate(float deltaTime);
    void SendGameState();
    void CheckCollisions();
    
    // Helper functions
    sf::Vector2f Lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t);
    sf::Vector2f ParseVector(const std::string& str);
    std::vector<std::string> SplitString(const std::string& s, char delimiter);
    bool CheckCollision(const sf::Vector2f& a, const sf::Vector2f& b, float radius);

    Role currentRole;
    GameState currentState;
    UdpNetworkManager& network;
    sf::Clock stateUpdateClock;
    float stateSendInterval = 0.05f;
    sf::Vector2f targetClientPosition;
};