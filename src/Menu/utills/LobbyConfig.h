#pragma once
#include <SFML/Graphics.hpp>
#include "Game.h"
#include "State.h"


struct LobbyConfig {
    GameDataRef data;
    int serverSocketForClient;
    int clientSocket;
    std::string hostName;
    std::string clientName;
    bool isHost;
};
