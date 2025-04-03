#pragma once
#include <SFML/Graphics.hpp>
#include "Game.h"
#include "State.h"

class MenuState : public State {
public:
    MenuState(GameDataRef data);
    ~MenuState();
    void Init() override;

    void HandleInput() override;
    void Update() override;
    void Draw() override;
    void ClearObjects() override;


private:
    GameDataRef _data;
    sf::Font _font;
    sf::Text *_titleText;
    sf::RectangleShape *_playButton;
    sf::Text *_playButtonText;
    
};
