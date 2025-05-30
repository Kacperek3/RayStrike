// GameEngine.cpp
#include "GameEngine.h"

GameEngine::GameEngine(Role role, UdpNetworkManager& network) 
    : currentRole(role), network(network) 
{
    currentState.hostPlayer.position = {400, 300};
    currentState.clientPlayer.position = {100, 300};
    currentState.hostPlayer.health = 100;
    currentState.clientPlayer.health = 100;
}

void GameEngine::Update(float deltaTime) {
    if(currentRole == Role::Host) {
        HostUpdate(deltaTime);
    } else {
        ClientUpdate(deltaTime);
    }
}
void GameEngine::AddHostBullet(const BulletState& bullet) {
    currentState.bullets.push_back(bullet);
    // Możesz dodać dodatkową logikę weryfikacji dla hosta
}

void GameEngine::AddClientBullet(const BulletState& bullet) {
    // Dla klienta możesz dodać logikę przewidywania
    currentState.bullets.push_back(bullet);
}

void GameEngine::HostUpdate(float deltaTime) {
    currentState.hostPlayer.position += currentState.hostPlayer.velocity * deltaTime;
    currentState.clientPlayer.position += currentState.clientPlayer.velocity * deltaTime;

    // Update bullets
    for(auto& bullet : currentState.bullets) {
        bullet.position += bullet.velocity * deltaTime;
        bullet.lifetime -= deltaTime;
    }

    // Remove expired bullets
    currentState.bullets.erase(
        std::remove_if(currentState.bullets.begin(), currentState.bullets.end(),
            [](const BulletState& b) { return b.lifetime <= 0.0f; }),
        currentState.bullets.end()
    );

    CheckCollisions();
    
    if(stateUpdateClock.getElapsedTime().asSeconds() >= stateSendInterval) {
        SendGameState();
        stateUpdateClock.restart();
    }
}

void GameEngine::CheckCollisions() {
    const float playerRadius = 20.0f;
    const float bulletRadius = 5.0f;

    for(auto& bullet : currentState.bullets) {
        if(std::hypot(bullet.position.x - currentState.hostPlayer.position.x, 
                     bullet.position.y - currentState.hostPlayer.position.y) < playerRadius + bulletRadius) {
            currentState.hostPlayer.health -= 10;
            network.Send("EVENT|HIT|HOST|10");
        }
        
        if(std::hypot(bullet.position.x - currentState.clientPlayer.position.x, 
                     bullet.position.y - currentState.clientPlayer.position.y) < playerRadius + bulletRadius) {
            currentState.clientPlayer.health -= 10;
            network.Send("EVENT|HIT|CLIENT|10");
        }
    }
}

void GameEngine::SendGameState() {
    std::stringstream ss;
    ss << "STATE|"
       << currentState.hostPlayer.position.x << "," << currentState.hostPlayer.position.y << "|"
       << currentState.clientPlayer.position.x << "," << currentState.clientPlayer.position.y << "|"
       << currentState.hostPlayer.health << "|"
       << currentState.clientPlayer.health;

    for(const auto& bullet : currentState.bullets) {
        ss << "|B|" << bullet.position.x << "," << bullet.position.y << "," 
           << bullet.velocity.x << "," << bullet.velocity.y;
    }
    
    network.Send(ss.str());
}

void GameEngine::ClientUpdate(float deltaTime) {
    // Interpolate client position
    currentState.clientPlayer.position = Lerp(
        currentState.clientPlayer.position,
        targetClientPosition,
        5.0f * deltaTime
    );

    // Update local prediction
    currentState.hostPlayer.position += currentState.hostPlayer.velocity * deltaTime;
}

void GameEngine::HandleInput(const sf::Event& event) {
    if(event.type == sf::Event::KeyPressed) {
        if(currentRole == Role::Host) {
            // Handle host input
            if(event.key.code == sf::Keyboard::W) currentState.hostPlayer.velocity.y = -currentState.hostPlayer.speed;
            if(event.key.code == sf::Keyboard::S) currentState.hostPlayer.velocity.y = currentState.hostPlayer.speed;
            if(event.key.code == sf::Keyboard::A) currentState.hostPlayer.velocity.x = -currentState.hostPlayer.speed;
            if(event.key.code == sf::Keyboard::D) currentState.hostPlayer.velocity.x = currentState.hostPlayer.speed;
        }
        else {
            // Handle client input
            if(event.key.code == sf::Keyboard::W) currentState.clientPlayer.velocity.y = -currentState.clientPlayer.speed;
            if(event.key.code == sf::Keyboard::S) currentState.clientPlayer.velocity.y = currentState.clientPlayer.speed;
            if(event.key.code == sf::Keyboard::A) currentState.clientPlayer.velocity.x = -currentState.clientPlayer.speed;
            if(event.key.code == sf::Keyboard::D) currentState.clientPlayer.velocity.x = currentState.clientPlayer.speed;
        }
    }
    else if(event.type == sf::Event::KeyReleased) {
        // Handle key releases
    }
}

void GameEngine::ReceiveNetworkMessage(const std::string& message) {
    std::vector<std::string> parts;
    std::stringstream ss(message);
    std::string part;
    
    while(std::getline(ss, part, '|')) {
        parts.push_back(part);
    }
    
    if(parts.empty()) return;

    if(parts[0] == "STATE") {
        if(parts.size() >= 5) {
            // Parse positions
            size_t comma = parts[1].find(',');
            currentState.hostPlayer.position.x = std::stof(parts[1].substr(0, comma));
            currentState.hostPlayer.position.y = std::stof(parts[1].substr(comma+1));
            
            comma = parts[2].find(',');
            currentState.clientPlayer.position.x = std::stof(parts[2].substr(0, comma));
            currentState.clientPlayer.position.y = std::stof(parts[2].substr(comma+1));
            
            // Parse health
            currentState.hostPlayer.health = std::stoi(parts[3]);
            currentState.clientPlayer.health = std::stoi(parts[4]);
            
            // Parse bullets
            currentState.bullets.clear();
            for(size_t i = 5; i < parts.size(); ++i) {
                if(parts[i] == "B" && i+2 < parts.size()) {
                    BulletState bullet;
                    comma = parts[i+1].find(',');
                    bullet.position.x = std::stof(parts[i+1].substr(0, comma));
                    bullet.position.y = std::stof(parts[i+1].substr(comma+1));
                    
                    comma = parts[i+2].find(',');
                    bullet.velocity.x = std::stof(parts[i+2].substr(0, comma));
                    bullet.velocity.y = std::stof(parts[i+2].substr(comma+1));
                    
                    currentState.bullets.push_back(bullet);
                    i += 2;
                }
            }
        }
    }
}

// Helper function implementations
sf::Vector2f GameEngine::Lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t) {
    return a + (b - a) * t;
}

sf::Vector2f GameEngine::ParseVector(const std::string& str) {
    size_t comma = str.find(',');
    if(comma == std::string::npos) return {0,0};
    return {
        std::stof(str.substr(0, comma)),
        std::stof(str.substr(comma+1))
    };
}

std::vector<std::string> GameEngine::SplitString(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while(std::getline(tokenStream, token, delimiter)) {
        if(!token.empty()) tokens.push_back(token);
    }
    return tokens;
}

bool GameEngine::CheckCollision(const sf::Vector2f& a, const sf::Vector2f& b, float radius) {
    return std::hypot(a.x - b.x, a.y - b.y) < radius;
}