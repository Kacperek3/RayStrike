#pragma once
#include <SFML/Graphics.hpp>
#include "Game.h"
#include "State.h"
#include <unordered_map>
#include <ctime>
#include <cmath>

#include "NetworkLobbyManager.h"
#include "LobbyConfig.h"
#include "GameConfig.h"
#include "Tesseract.h"



class LobbyState : public State {
public:
    LobbyState(const LobbyConfig config);
    ~LobbyState();
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


    LobbyConfig _config;
    GameSettings _gameSettings;

    GameDataRef _data;
    AnimationState _animationState = AnimationState::ENTERING;
    std::unordered_map<sf::RectangleShape*, std::tuple<sf::Vector2f, sf::Vector2f, sf::FloatRect, sf::Color>> _buttonData;


    sf::Font _font;
    sf::Sprite *_backgroundTexture;
    sf::Sprite *_hostIcon;
    sf::Sprite *_plusNumOfRoundsIcon;
    sf::Sprite *_minusNumOfRoundsIcon;
    sf::Sprite *_plusTimeLimitIcon;
    sf::Sprite *_minusTimeLimitIcon;


    sf::Sprite *_clientIcon;
    sf::Text *_titleText;

    sf::RectangleShape *_backgroundPlayerList;
    sf::RectangleShape *_backgroundPlayerListPanel;
    sf::RectangleShape *_spacer;
    sf::Text *_tittlePlayerList;

    sf::RectangleShape *_backgroundForHostList;
    sf::Text *_hostNameText;
    sf::Text *_hostHintText;

    sf::RectangleShape *_backgroundForClientList;
    sf::Text *_clientNameText;
    sf::Text *_clientHintText;


    sf::RectangleShape *_backgroundInfoList;
    sf::RectangleShape *_backgroundInfoListPanel;
    sf::RectangleShape *_spacerInfoList;
    sf::Text *_tittleInfoList;

    sf::RectangleShape *_backgroundForNumberOfRounds;
    sf::Text *_numberOfRoundsText;
    sf::RectangleShape *_backgroundForTimeLimit;
    sf::Text *_timeLimitText;

    sf::RectangleShape *_startGameButton;
    sf::Text *_startGameButtonText;
    sf::RectangleShape *_configureButton;
    sf::Text *_configureButtonText;
    sf::RectangleShape *_backButton;
    sf::Text *_backButtonText;

    
    Tesseract *_tesseract;
    

    const float _exitAnimationSpeed = 80.0f;


    // network
    NetworkLobbyManager *_networkLobbyManager;

};
