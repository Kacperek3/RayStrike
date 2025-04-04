#include "Game.h"
#include "MenuState.h"

Game::Game(int width, int height, std::string title) {
    _data->windowedWidth = width;
    _data->windowedHeight = height;
    _data->windowTitle = title;
    _data->originalResolution = {this->_data->windowedWidth, this->_data->windowedHeight};

    _data->window.create(sf::VideoMode(width, height), title, sf::Style::Close | sf::Style::Titlebar);
    _data->window.setFramerateLimit(60);

    sf::View view(sf::FloatRect(0, 0, width, height));
    _data->window.setView(view);

    _data->stateManager.AddState(StateRef(new MenuState(this->_data)));
    this->Run();
}

void Game::Run() {
    while (this->_data->window.isOpen()) {
        this->_data->stateManager.ProcessStateChanges();
        this->_data->stateManager.GetActiveState()->HandleInput();
        this->_data->stateManager.GetActiveState()->Update();
        this->_data->stateManager.GetActiveState()->Draw();
    }
}