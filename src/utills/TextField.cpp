#include "TextField.h"

TextField::TextField(float x, float y, float width, float height, sf::Font& font, unsigned int charSize) : isActive(false){
    box.setPosition(x, y);
    box.setSize({ width, height });
    box.setFillColor(sf::Color::White);
    box.setOutlineThickness(4);
    box.setOutlineColor(sf::Color::Black);

    text.setFont(font);
    text.setCharacterSize(charSize);
    text.setFillColor(sf::Color::Black);
    text.setPosition(x + 5, y + 5);

    defaultOutlineColor = sf::Color::Black;
}

void TextField::setPosition(float x, float y) {
    box.setPosition(x, y);
    text.setPosition(x + 5, y + 5);
}
sf::FloatRect TextField::getGlobalBounds() const {
    return box.getGlobalBounds();
}
sf::Vector2f TextField::getPosition() const {
    return box.getPosition();
}


void TextField::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
        isActive = box.getGlobalBounds().contains(mousePos);
        box.setOutlineColor(isActive ? sf::Color(80, 150, 255,150) : defaultOutlineColor);
    }
    else if (isActive && event.type == sf::Event::TextEntered) {
        if (event.text.unicode == 8) {
            if (!input.empty()) input.pop_back();
        }
        else if (event.text.unicode >= 32 && event.text.unicode < 128) {
            input += static_cast<char>(event.text.unicode);
        }
        text.setString(input);
    }
}


void TextField::draw(sf::RenderWindow& window) {
    window.draw(box);
    window.draw(text);
}


 std::string TextField::getInput() const {
    return input;
}
