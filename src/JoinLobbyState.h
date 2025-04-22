#pragma once
#include <SFML/Graphics.hpp>
#include "Game.h"
#include "State.h"
#include <unordered_map>
#include <ctime>
#include <cmath>

#include "NetworkManager.h"



class JoinLobbyState : public State {
public:
    JoinLobbyState(GameDataRef data);
    ~JoinLobbyState();
    void Init() override;

    void HandleInput() override;
    void Update() override;
    void Draw() override;
    void ClearObjects() override;


private:

    enum class AnimationState {
        ENTERING,
        EXITING,
        NONE
    };


    void exitingAnimation();
    void enteringAnimation();
    void standartAnimation();


    GameDataRef _data;
    AnimationState _animationState = AnimationState::ENTERING;

    sf::Font _font;
    sf::Sprite *_backgroundTexture;
    sf::Text *_titleText;

    sf::RectangleShape *_backgroundForLobbyEntries;
    sf::RectangleShape *_backgroundForLobbyEntriesPanel;
    sf::Sprite *_reloadLobbiesButton;
    sf::Text *_tittleNameLobby;
    sf::Text *_tittleNamePlayer;
    sf::Text *_tittleIpLobby;
    

    const float _exitAnimationSpeed = 80.0f;

    std::vector<sf::Text*> lobbyEntries;
    std::vector<LobbyInfo> currentLobbies;
    NetworkManager _networkManager;


};
