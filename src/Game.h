#pragma once

#include <memory>
#include <string>
#include <SFML/Graphics.hpp>
#include "GameStateManager.h"
#include "AssetManager.h"
#include "InputManager.h"
#include "SoundManager.h"


struct GameData {
    GameStateManager stateManager;
    sf::RenderWindow window;
    AssetManager assetManager;
    InputManager inputManager;
    SoundManager soundManager;
};

typedef std::shared_ptr<GameData> GameDataRef;

class Game {
public:
    Game(int width, int height, std::string title);
private:
    GameDataRef _data = std::make_shared<GameData>();
    void Run();
};