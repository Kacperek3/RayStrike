#pragma once
#include <SFML/Graphics.hpp>
#include "Game.h"
#include "State.h"

struct GameSettings{
    int numberOfRounds;
    int timeLimit;
};


struct GameConfig {
    GameDataRef data;
    int serverSocketForClient;
    int clientSocket;
    std::string hostName;
    std::string clientName;
    bool isHost;
};
