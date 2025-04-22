#include "JoinLobbyState.h"

JoinLobbyState::JoinLobbyState(GameDataRef data) : _data(data) {
    srand(static_cast<unsigned>(time(NULL)));

    _backgroundTexture = new sf::Sprite();
    _reloadLobbiesButton = new sf::Sprite();
    _titleText = new sf::Text();
    _backgroundForLobbyEntries = new sf::RectangleShape();
    _backgroundForLobbyEntriesPanel = new sf::RectangleShape();
    _tittleNameLobby = new sf::Text();
    _tittleNamePlayer = new sf::Text();
    _tittleIpLobby = new sf::Text();
}

void JoinLobbyState::Init(){
    if (!_font.loadFromFile("assets/fonts/Orbitron/Orbitron-VariableFont_wght.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }
    

    _data->assetManager.LoadTexture("background", "assets/background.jpg");
    _backgroundTexture->setTexture(_data->assetManager.GetTexture("background"));
    _data->assetManager.LoadTexture("reloadLobbiesButton", "assets/flair_arrow_3.png");

    _titleText->setFont(_font);
    _titleText->setString("Join Lobby");
    _titleText->setCharacterSize(50);
    _titleText->setFillColor(sf::Color::White);


    _titleText->setPosition(_data->window.getSize().x,60);


    _backgroundForLobbyEntries->setSize(sf::Vector2f(950, 600));
    _backgroundForLobbyEntries->setFillColor(sf::Color(58, 58, 58, 200));
    _backgroundForLobbyEntries->setPosition(165, 200);

    _backgroundForLobbyEntriesPanel->setSize(sf::Vector2f(950,65));
    _backgroundForLobbyEntriesPanel->setFillColor(sf::Color(88, 88, 88, 200));
    _backgroundForLobbyEntriesPanel->setPosition(165, 200);

    _reloadLobbiesButton->setTexture(_data->assetManager.GetTexture("reloadLobbiesButton"));
    _reloadLobbiesButton->setPosition(1000, 202);

    _tittleNameLobby->setFont(_font);
    _tittleNameLobby->setString("Lobby Name");
    _tittleNameLobby->setCharacterSize(23);
    _tittleNameLobby->setFillColor(sf::Color::White);
    _tittleNameLobby->setPosition(200, 218);

    _tittleNamePlayer->setFont(_font);
    _tittleNamePlayer->setString("Player Name");
    _tittleNamePlayer->setCharacterSize(23);
    _tittleNamePlayer->setFillColor(sf::Color::White);
    _tittleNamePlayer->setPosition(480, 218);

    _tittleIpLobby->setFont(_font);
    _tittleIpLobby->setString("IP");
    _tittleIpLobby->setCharacterSize(23);
    _tittleIpLobby->setFillColor(sf::Color::White);
    _tittleIpLobby->setPosition(800, 218);


    _networkManager.startLobbyDiscovery(8888);




    auto lobbies = _networkManager.getDiscoveredLobbies();
    currentLobbies.clear();
    
    // Sortowanie lobby według czasu ostatniej aktywności
    std::vector<LobbyInfo> temp;
    for(auto& pair : lobbies) {
        temp.push_back(pair.second);
    }
    
    // Sortowanie dla spójności wyświetlania
    std::sort(temp.begin(), temp.end(), [](const LobbyInfo& a, const LobbyInfo& b) {
        return a.lastSeen > b.lastSeen; 
    });
    
    currentLobbies = temp;
    
    // Aktualizuj UI
    const float startY = 200.0f;
    const float spacing = 60.0f;
    
    // Usuń stare wpisy
    for(auto entry : lobbyEntries) delete entry;
    lobbyEntries.clear();
    
    // Stwórz nowe wpisy
    for(size_t i = 0; i < currentLobbies.size(); ++i) {
        std::cout << currentLobbies.size() << std::endl;
        auto entry = new sf::Text();
        entry->setFont(_font);
        entry->setString(currentLobbies[i].name + " (" + currentLobbies[i].ip + ")");
        entry->setCharacterSize(30);
        entry->setFillColor(sf::Color::White);
        entry->setPosition(100, startY + i * spacing);
        lobbyEntries.push_back(entry);
    }
   
}

void JoinLobbyState::HandleInput() {
    sf::Event event;
    while (_data->window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _data->window.close();
            return;
        }
        if(event.type == sf::Event::MouseButtonPressed ) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = _data->inputManager.GetMousePosition(_data->window);
                

                if(_data->inputManager.IsSpriteClicked(*_reloadLobbiesButton, sf::Mouse::Left, _data->window)) {
                    _networkManager.stopLobbyDiscovery();
                    _networkManager.startLobbyDiscovery(8888);
                    auto lobbies = _networkManager.getDiscoveredLobbies();
                    currentLobbies.clear();
                    
                    std::vector<LobbyInfo> temp;
                    for(auto& pair : lobbies) {
                        temp.push_back(pair.second);
                    }
                    
                    std::sort(temp.begin(), temp.end(), [](const LobbyInfo& a, const LobbyInfo& b) {
                        return a.lastSeen > b.lastSeen; 
                    });
                    
                    currentLobbies = temp;
                    
                    // Aktualizuj UI
                    const float startY = 300.0f;
                    const float spacing = 60.0f;
                    
                    for(auto entry : lobbyEntries) delete entry;
                    lobbyEntries.clear();
                    
                    for(size_t i = 0; i < currentLobbies.size(); ++i) {
                        auto entry = new sf::Text();
                        entry->setFont(_font);
                        entry->setString(currentLobbies[i].name + "      " + currentLobbies[i].playerName + "        " + currentLobbies[i].ip);
                        entry->setCharacterSize(19);
                        entry->setFillColor(sf::Color::White);
                        entry->setPosition(200, startY + i * spacing);
                        lobbyEntries.push_back(entry);
                    }
                }
            }
            return;
        }
        if(event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                _animationState = AnimationState::EXITING;
            }
        }
    }
}

void JoinLobbyState::Update() {

    // Animation logic
    if(_animationState == AnimationState::ENTERING) {
        enteringAnimation();
    } else if(_animationState == AnimationState::EXITING) {
        exitingAnimation();
    } else {
        standartAnimation();
    }


}


void JoinLobbyState::exitingAnimation() {
    
    sf::Vector2f currentPos = _titleText->getPosition();

    if (currentPos.x < _data->window.getSize().x) {
        currentPos.x += _exitAnimationSpeed;
        if (currentPos.x >= _data->window.getSize().x){
            currentPos.x = _data->window.getSize().x;
            _animationState = AnimationState::ENTERING;
            _data->stateManager.RemoveState();
        }
        
        _titleText->setPosition(currentPos);

    }
    
    return;
}

void JoinLobbyState::enteringAnimation() {
    sf::Vector2f currentPos = _titleText->getPosition();

    if (currentPos.x > 0) {
        currentPos.x -= _exitAnimationSpeed;
        if (currentPos.x <= 500){
            currentPos.x = 500;
            _animationState = AnimationState::NONE;
        }
        
        _titleText->setPosition(currentPos);

    }
}

void JoinLobbyState::standartAnimation(){
    return;
}




void JoinLobbyState::Draw() {
    _data->window.clear();
    _data->window.draw(*_backgroundTexture);
    _data->window.draw(*_backgroundForLobbyEntries);
    _data->window.draw(*_backgroundForLobbyEntriesPanel);
    _data->window.draw(*_tittleNameLobby);
    _data->window.draw(*_tittleNamePlayer);
    _data->window.draw(*_tittleIpLobby);
    _data->window.draw(*_reloadLobbiesButton);
    _data->window.draw(*_titleText);

    for(auto entry : lobbyEntries) {
        _data->window.draw(*entry);
        //std::cout << entry->getString().toAnsiString() << std::endl;
    }

    _data->window.display();
}


JoinLobbyState::~JoinLobbyState() {
    delete _backgroundTexture;
    delete _backgroundForLobbyEntries;
    delete _backgroundForLobbyEntriesPanel;
    delete _tittleNameLobby;
    delete _tittleNamePlayer;
    delete _tittleIpLobby;
    delete _reloadLobbiesButton;
    delete _titleText;
    _networkManager.stopLobbyDiscovery();
    for(auto entry : lobbyEntries) delete entry;
    lobbyEntries.clear();
}


void JoinLobbyState::ClearObjects() {
   // _data->assetManager.clearAssets();
    //_data->soundManager.ClearSounds();
}