#include "LobbyState.h"

LobbyState::LobbyState(const LobbyConfig config) : _config(config), _data(config.data) {
    srand(static_cast<unsigned>(time(NULL)));

    _backgroundTexture = new sf::Sprite();
    _hostIcon = new sf::Sprite();
    _clientIcon = new sf::Sprite();
    _plusNumOfRoundsIcon = new sf::Sprite();
    _minusNumOfRoundsIcon = new sf::Sprite();
    _plusTimeLimitIcon = new sf::Sprite();
    _minusTimeLimitIcon = new sf::Sprite();


    _backgroundPlayerList = new sf::RectangleShape();
    _backgroundPlayerListPanel = new sf::RectangleShape();
    _spacer = new sf::RectangleShape();
    _tittlePlayerList = new sf::Text();

    _backgroundForHostList = new sf::RectangleShape();
    _hostNameText = new sf::Text();
    _hostHintText = new sf::Text();

    _backgroundForClientList = new sf::RectangleShape();
    _clientNameText = new sf::Text();
    _clientHintText = new sf::Text();


    _backgroundInfoList = new sf::RectangleShape();
    _backgroundInfoListPanel = new sf::RectangleShape();
    _spacerInfoList = new sf::RectangleShape();
    _tittleInfoList = new sf::Text();
    _backgroundForNumberOfRounds = new sf::RectangleShape();
    _backgroundForTimeLimit = new sf::RectangleShape();
    _numberOfRoundsText = new sf::Text();
    _timeLimitText = new sf::Text();


    _titleText = new sf::Text();

    _tesseract = new Tesseract();


    _startGameButton = new sf::RectangleShape();
    _startGameButtonText = new sf::Text();
    _configureButton = new sf::RectangleShape();
    _configureButtonText = new sf::Text();
    _backButton = new sf::RectangleShape();
    _backButtonText = new sf::Text();
    
    _networkLobbyManager = new NetworkLobbyManager(_config.serverSocketForClient, _config.clientSocket);
}

void LobbyState::Init(){
    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }



    _data->assetManager.LoadTexture("background", "assets/background.jpg");
    _backgroundTexture->setTexture(_data->assetManager.GetTexture("background"));
    _backgroundTexture->setPosition(0, 0);

    _data->assetManager.LoadTexture("crownIcon", "assets/hosticon.jpg");
    _hostIcon->setTexture(_data->assetManager.GetTexture("crownIcon"));
    _hostIcon->setPosition(790, 250);

    _data->assetManager.LoadTexture("clientIcon", "assets/clientIcon.jpg");
    _clientIcon->setTexture(_data->assetManager.GetTexture("clientIcon"));
    _clientIcon->setPosition(790, 313);

    _data->assetManager.LoadTexture("plusIcon", "assets/flair_plus.png");
    _plusNumOfRoundsIcon->setTexture(_data->assetManager.GetTexture("plusIcon"));
    _plusNumOfRoundsIcon->setPosition(580, 570);
    _data->assetManager.LoadTexture("minusIcon", "assets/flair_minus.png");
    _minusNumOfRoundsIcon->setTexture(_data->assetManager.GetTexture("minusIcon"));
    _minusNumOfRoundsIcon->setPosition(180, 570);

    _data->assetManager.LoadTexture("plusIcon", "assets/flair_plus.png");
    _plusTimeLimitIcon->setTexture(_data->assetManager.GetTexture("plusIcon"));
    _plusTimeLimitIcon->setPosition(580, 633);
    _data->assetManager.LoadTexture("minusIcon", "assets/flair_minus.png");
    _minusTimeLimitIcon->setTexture(_data->assetManager.GetTexture("minusIcon"));
    _minusTimeLimitIcon->setPosition(180, 633);

    _backgroundPlayerList->setSize(sf::Vector2f(400, 650));
    _backgroundPlayerList->setFillColor(sf::Color(58, 58, 58, 200));
    _backgroundPlayerList->setPosition(780, 170);
    _backgroundPlayerListPanel->setSize(sf::Vector2f(400, 65));
    _backgroundPlayerListPanel->setFillColor(sf::Color(88, 88, 88, 200));
    _backgroundPlayerListPanel->setPosition(780, 170);
    _spacer->setSize(sf::Vector2f(400, 5));
    _spacer->setFillColor(sf::Color(255, 255, 255, 200));
    _spacer->setPosition(780, 235);
    _tittlePlayerList->setFont(_font);
    _tittlePlayerList->setString("Player List");
    _tittlePlayerList->setCharacterSize(23);
    _tittlePlayerList->setFillColor(sf::Color::White);
    _tittlePlayerList->setPosition(900, 188);

    _backgroundForHostList->setSize(sf::Vector2f(400, 65));
    _backgroundForHostList->setFillColor(sf::Color(115, 123, 146 , 150));
    _backgroundForHostList->setPosition(780, 240);
    _hostNameText->setFont(_font);
    _hostNameText->setString(_config.hostName);
    _hostNameText->setCharacterSize(19);
    _hostNameText->setFillColor(sf::Color(255, 223, 0));
    _hostNameText->setPosition(860, 258);
    _hostHintText->setFont(_font);
    _hostHintText->setString("Host");
    _hostHintText->setCharacterSize(16);
    _hostHintText->setFillColor(sf::Color(230, 230, 230,100));
    _hostHintText->setPosition(1030, 258);

    _backgroundForClientList->setSize(sf::Vector2f(400, 65));
    _backgroundForClientList->setFillColor(sf::Color(115, 123, 146 , 150));
    _backgroundForClientList->setPosition(780, 303);
    _clientNameText->setFont(_font);
    _clientNameText->setString(_config.clientName);
    _clientNameText->setCharacterSize(19);
    _clientNameText->setFillColor(sf::Color(230, 230, 230));
    _clientNameText->setPosition(860, 321);
    _clientHintText->setFont(_font);
    _clientHintText->setString("Client");
    _clientHintText->setCharacterSize(16);
    _clientHintText->setFillColor(sf::Color(230, 230, 230,100));
    _clientHintText->setPosition(1030, 321);


    _backgroundInfoList->setSize(sf::Vector2f(500, 320));
    _backgroundInfoList->setFillColor(sf::Color(58, 58, 58, 200));
    _backgroundInfoList->setPosition(150, 500);
    _backgroundInfoListPanel->setSize(sf::Vector2f(500, 65));
    _backgroundInfoListPanel->setFillColor(sf::Color(88, 88, 88, 200));
    _backgroundInfoListPanel->setPosition(150, 500);
    _spacerInfoList->setSize(sf::Vector2f(500, 5));
    _spacerInfoList->setFillColor(sf::Color(255, 255, 255, 200));
    _spacerInfoList->setPosition(150, 565);
    _tittleInfoList->setFont(_font);
    _tittleInfoList->setString("Game Info");
    _tittleInfoList->setCharacterSize(23);
    _tittleInfoList->setFillColor(sf::Color::White);
    _tittleInfoList->setPosition(330, 513);

    _backgroundForNumberOfRounds->setSize(sf::Vector2f(500, 65));
    _backgroundForNumberOfRounds->setFillColor(sf::Color(115, 123, 146 , 150));
    _backgroundForNumberOfRounds->setPosition(150, 570);
    _numberOfRoundsText->setFont(_font);
    _gameSettings.numberOfRounds = 3;
    _numberOfRoundsText->setString("Number of rounds: " + std::to_string(_gameSettings.numberOfRounds));
    _numberOfRoundsText->setCharacterSize(19);
    _numberOfRoundsText->setFillColor(sf::Color(230, 230, 230, 230));
    _numberOfRoundsText->setPosition(290, 588);
    _backgroundForTimeLimit->setSize(sf::Vector2f(500, 65));
    _backgroundForTimeLimit->setFillColor(sf::Color(115, 123, 146 , 150));
    _backgroundForTimeLimit->setPosition(150, 633);
    _timeLimitText->setFont(_font);
    _gameSettings.timeLimit = 60;
    _timeLimitText->setString("Time limit: " + std::to_string(_gameSettings.timeLimit)+ " sec");
    _timeLimitText->setCharacterSize(19);
    _timeLimitText->setFillColor(sf::Color(230, 230, 230, 230));
    _timeLimitText->setPosition(300, 651);


    _titleText->setFont(_font);
    _titleText->setString("Lobby");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(sf::Color::White);
    _titleText->setPosition(_data->window.getSize().x,60);


    _startGameButton->setSize(sf::Vector2f(250, 50));
    _startGameButton->setFillColor(sf::Color(80, 150, 255,150));
    _startGameButton->setPosition(sf::Vector2f(280, 170));
    _startGameButtonText->setFont(_font);
    _startGameButtonText->setString("Start Game");
    _startGameButtonText->setCharacterSize(30);
    _startGameButtonText->setFillColor(sf::Color::White);
    _startGameButtonText->setPosition(_startGameButton->getPosition().x + 35, _startGameButton->getPosition().y + 8);

    _configureButton->setSize(sf::Vector2f(250, 50));
    _configureButton->setFillColor(sf::Color(80, 150, 255,150));
    _configureButton->setPosition(sf::Vector2f(280, 260));
    _configureButtonText->setFont(_font);
    _configureButtonText->setString("Configure");
    _configureButtonText->setCharacterSize(30);
    _configureButtonText->setFillColor(sf::Color::White);
    _configureButtonText->setPosition(_configureButton->getPosition().x + 45, _configureButton->getPosition().y + 8);

    _backButton->setSize(sf::Vector2f(250, 50));
    _backButton->setFillColor(sf::Color(80, 150, 255,150));
    _backButton->setPosition(sf::Vector2f(280, 350));
    _backButtonText->setFont(_font);
    _backButtonText->setString("Back");
    _backButtonText->setCharacterSize(30);
    _backButtonText->setFillColor(sf::Color::White);
    _backButtonText->setPosition(_backButton->getPosition().x + 70, _backButton->getPosition().y + 8);

    _tesseract->setPosition(sf::Vector2f(980, 270));
    _tesseract->setScale(0.15f);
    _tesseract->setColor(sf::Color(255, 223, 0,120));

    auto storeButtonData = [&](sf::RectangleShape* btn, sf::Text* txt) {
        sf::Vector2f originalBtnPos = btn->getPosition();
        sf::Vector2f originalTxtPos = txt->getPosition();
        sf::FloatRect originalBounds = btn->getGlobalBounds();
        sf::Color originalColor = btn->getFillColor();
        _buttonData[btn] = {originalBtnPos, originalTxtPos, originalBounds, originalColor};
    };
    storeButtonData(_backButton, _backButtonText);  
    storeButtonData(_startGameButton, _startGameButtonText);
    storeButtonData(_configureButton, _configureButtonText);
    

    _backButton->setPosition(1100, 700);
    _backButtonText->setPosition(1170, 708);

}

void LobbyState::HandleInput() {
    sf::Event event;
    while (_data->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _data->window.close();
            return;
        }
        if(event.type == sf::Event::MouseButtonPressed ) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
                if (_backButton->getGlobalBounds().contains(mousePos)) {
                    _animationState = AnimationState::EXITING;
                    _networkLobbyManager->Send("__DISCONNECT__");
                }
                else if (_startGameButton->getGlobalBounds().contains(mousePos)) {
                    _animationState = AnimationState::EXITING;
                    _networkLobbyManager->Send("__START_GAME__");
                }
                else if (_configureButton->getGlobalBounds().contains(mousePos)) {
                    _animationState = AnimationState::EXITING;
                    _networkLobbyManager->Send("__CONFIGURE__");
                }
                else if(_plusNumOfRoundsIcon->getGlobalBounds().contains(mousePos) && _config.isHost){
                    if(_gameSettings.numberOfRounds < 7){
                        _gameSettings.numberOfRounds++;
                        _numberOfRoundsText->setString("Number of rounds: " + std::to_string(_gameSettings.numberOfRounds));
                        _networkLobbyManager->Send("__NUMBER_OF_ROUNDS__" + std::to_string(_gameSettings.numberOfRounds));
                    }
                }
                else if(_minusNumOfRoundsIcon->getGlobalBounds().contains(mousePos) && _config.isHost){
                    if(_gameSettings.numberOfRounds > 1){
                        _gameSettings.numberOfRounds--;
                        _numberOfRoundsText->setString("Number of rounds: " + std::to_string(_gameSettings.numberOfRounds));
                        _networkLobbyManager->Send("__NUMBER_OF_ROUNDS__" + std::to_string(_gameSettings.numberOfRounds));
                    }
                }
                else if(_plusTimeLimitIcon->getGlobalBounds().contains(mousePos) && _config.isHost){
                    if(_gameSettings.timeLimit < 120){
                        _gameSettings.timeLimit+=5;
                        _timeLimitText->setString("Time limit: " + std::to_string(_gameSettings.timeLimit)+ " sec");
                        _networkLobbyManager->Send("__TIME_LIMIT__" + std::to_string(_gameSettings.timeLimit));
                    }
                }
                else if(_minusTimeLimitIcon->getGlobalBounds().contains(mousePos) && _config.isHost){
                    if(_gameSettings.timeLimit > 45){
                        _gameSettings.timeLimit-=5;
                        _timeLimitText->setString("Time limit: " + std::to_string(_gameSettings.timeLimit) + " sec");
                        _networkLobbyManager->Send("__TIME_LIMIT__" + std::to_string(_gameSettings.timeLimit));
                    }
                }
            }
            return;
        }
    }
}

void LobbyState::Update() {
    _tesseract->update();
    // Animation logic
    if(_animationState == AnimationState::ENTERING) {
        enteringAnimation();
    } else if(_animationState == AnimationState::EXITING) {
        exitingAnimation();
    } else {
        standartAnimation();
    }

    std::string msg;
    while (_networkLobbyManager->WaitForMessage(msg, 0)) {
        if (msg.find("__DISCONNECT") != std::string::npos) {
            _animationState = AnimationState::EXITING;
            break;
        }
        else if (msg.find("__NUMBER_OF_ROUNDS__") != std::string::npos) {
            std::string prefix = "__NUMBER_OF_ROUNDS__";
            size_t pos = msg.find(prefix);
            if (pos != std::string::npos) {
                std::string numberStr = msg.substr(pos + prefix.length());
                try {
                    int newRounds = std::stoi(numberStr);
                    _gameSettings.numberOfRounds = newRounds;
                    _numberOfRoundsText->setString("Number of rounds: " + std::to_string(newRounds));
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing number of rounds: " << e.what() << std::endl;
                }
            }
        }
        else if (msg.find("__TIME_LIMIT__") != std::string::npos) {
            std::string prefix = "__TIME_LIMIT__";
            size_t pos = msg.find(prefix);
            if (pos != std::string::npos) {
                std::string numberStr = msg.substr(pos + prefix.length());
                try {
                    int newTimeLimit = std::stoi(numberStr);
                    _gameSettings.timeLimit = newTimeLimit;
                    _timeLimitText->setString("Time limit: " + std::to_string(newTimeLimit)+ " sec");
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing time limit: " << e.what() << std::endl;
                }
            }
        }
    }
}


void LobbyState::exitingAnimation() {
    

    sf::Vector2f currentPos = _titleText->getPosition();

    if (currentPos.x < _data->window.getSize().x) {
        currentPos.x += _exitAnimationSpeed;
        _titleText->setPosition(currentPos);
    }
    if (currentPos.x >= _data->window.getSize().x) {
        currentPos.x = _data->window.getSize().x;
        _animationState = AnimationState::ENTERING;
        _data->stateManager.RemoveState();
    }
    
    return;
}

void LobbyState::enteringAnimation() {
    bool allButtonsOffScreen = true;
    bool allTextFieldsOffScreen = true;


    for (auto& [button, data] : _buttonData) {
        auto& [originalBtnPos, originalTxtPos, originalBounds, originalColor] = data;
        
        sf::Vector2f newPos = button->getPosition();
        newPos.x -= _exitAnimationSpeed;
        button->setPosition(newPos);
        _titleText->setPosition(_titleText->getPosition().x - _exitAnimationSpeed, _titleText->getPosition().y);


        sf::Text* text = nullptr;
        if (button == _backButton) text = _backButtonText;
        
        if (text) {
            sf::Vector2f textPos = text->getPosition();
            textPos.x -= _exitAnimationSpeed;
            text->setPosition(textPos);
        }

        sf::FloatRect bounds = button->getGlobalBounds();
        if (bounds.left + bounds.width > 0) {
            allButtonsOffScreen = false;
        }
    }


    
    sf::Vector2f currentPos = _titleText->getPosition();

    if (currentPos.x > 0) {
        currentPos.x -= _exitAnimationSpeed;
        if (currentPos.x <= 550){
            currentPos.x = 550;
            _animationState = AnimationState::NONE;
        }
        
        _titleText->setPosition(currentPos);

    }
}

void LobbyState::standartAnimation(){
    sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
    const float hoverOffset = 10.f;
    const sf::Color hoverColor(0, 59, 190);
    for (auto& [button, data] : _buttonData) {
        auto& [originalBtnPos, originalTxtPos, originalBounds, originalColor] = data;
        sf::Text* text = nullptr;

        if (button == _backButton) text = _backButtonText;
        else if (button == _startGameButton) text = _startGameButtonText;
        else if (button == _configureButton) text = _configureButtonText;
        
        if (originalBounds.contains(mousePos)) {
            button->setPosition(originalBtnPos.x, originalBtnPos.y);
            button->setFillColor(hoverColor);
            if (text) text->setPosition(originalTxtPos.x, originalTxtPos.y);
        } else {
            button->setPosition(originalBtnPos);
            button->setFillColor(originalColor);
            if (text) text->setPosition(originalTxtPos);
        }
    }
}


void LobbyState::Draw() {
    _data->window.clear();
    _data->window.draw(*_backgroundTexture);
    _data->window.draw(*_backgroundPlayerList);
    _data->window.draw(*_backgroundPlayerListPanel);
    _data->window.draw(*_spacer);
    _data->window.draw(*_tittlePlayerList);
    _data->window.draw(*_backgroundForHostList);
    _data->window.draw(*_hostNameText);
    _data->window.draw(*_hostHintText);
    _data->window.draw(*_backgroundForClientList);
    _data->window.draw(*_clientNameText);
    _data->window.draw(*_clientHintText);

    _data->window.draw(*_backgroundInfoList);
    _data->window.draw(*_backgroundInfoListPanel);
    _data->window.draw(*_spacerInfoList);
    _data->window.draw(*_tittleInfoList);
    _data->window.draw(*_backgroundForNumberOfRounds);
    _data->window.draw(*_numberOfRoundsText);
    _data->window.draw(*_backgroundForTimeLimit);
    _data->window.draw(*_timeLimitText);

    _tesseract->draw(_data->window);

    _data->window.draw(*_titleText);

    _data->window.draw(*_startGameButton);
    _data->window.draw(*_startGameButtonText);
    _data->window.draw(*_configureButton);
    _data->window.draw(*_configureButtonText);
    _data->window.draw(*_backButton);
    _data->window.draw(*_backButtonText);


    _data->window.draw(*_hostIcon);
    _data->window.draw(*_clientIcon);

    if(_config.isHost){
        _data->window.draw(*_plusNumOfRoundsIcon);
        _data->window.draw(*_minusNumOfRoundsIcon);
        _data->window.draw(*_plusTimeLimitIcon);
        _data->window.draw(*_minusTimeLimitIcon);
    }
    _data->window.display();
}


LobbyState::~LobbyState() {
    delete _backgroundTexture;
    delete _hostIcon;
    delete _clientIcon;
    delete _plusNumOfRoundsIcon;
    delete _minusNumOfRoundsIcon;
    delete _plusTimeLimitIcon;
    delete _minusTimeLimitIcon;
    delete _backgroundPlayerList;
    delete _backgroundPlayerListPanel;
    delete _spacer;
    delete _tittlePlayerList;
    delete _backgroundForHostList;
    delete _hostNameText;
    delete _hostHintText;
    delete _backgroundForClientList;
    delete _clientNameText;
    delete _clientHintText;
    delete _backgroundInfoList;
    delete _backgroundInfoListPanel;
    delete _spacerInfoList;
    delete _tittleInfoList;
    delete _backgroundForNumberOfRounds;
    delete _backgroundForTimeLimit;
    delete _numberOfRoundsText;
    delete _timeLimitText;
    delete _tesseract;
    delete _titleText;
    delete _startGameButton;
    delete _startGameButtonText;
    delete _configureButton;
    delete _configureButtonText;
    delete _backButton;
    delete _backButtonText;
    delete _networkLobbyManager;

}


void LobbyState::ClearObjects() {
   // _data->assetManager.clearAssets();
    //_data->soundManager.ClearSounds();
}