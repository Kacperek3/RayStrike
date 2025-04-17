#pragma once

#include <SFML/Graphics.hpp>
#include "TextField.h"
#include <string>

class LabeledTextField {
public:
    LabeledTextField(float x, float y, float width, float height,int boxWidth ,const std::string& label, sf::Font &font);
    ~LabeledTextField();

    void setPosition(sf::Vector2f position);
    void move(sf::Vector2f offset);
    sf::Vector2f getPosition() const;

    void handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window);

    std::string getText() {
        return _textField->getInput();
    }

private:
    TextField *_textField;
    sf::Text *_labelText;
    sf::RectangleShape *_labelBox;

    sf::Vector2f _position;
};
