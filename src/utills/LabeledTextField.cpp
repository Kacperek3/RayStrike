#include "LabeledTextField.h"

LabeledTextField::LabeledTextField(float x, float y, float width, float height,int boxWidth, const std::string& label, sf::Font &font)
    :  _position(x, y)
{
    _textField = new TextField(x, y, width, height, font);

    _labelText = new sf::Text();
    _labelText->setFont(font);
    _labelText->setString(label);
    _labelText->setCharacterSize(15);
    _labelText->setFillColor(sf::Color::White);
    _labelText->setPosition(x + 6, y - 29);

    _labelBox = new sf::RectangleShape();
    _labelBox->setSize(sf::Vector2f(boxWidth, 27));
    _labelBox->setFillColor(sf::Color(80, 150, 255, 150));
    _labelBox->setPosition(x - 2, y - 34);
}

void LabeledTextField::setPosition(sf::Vector2f position) {
    _position = position;
    _textField->setPosition(position);
    _labelText->setPosition(position.x + 6, position.y - 29);
    _labelBox->setPosition(position.x - 2, position.y - 34);
}

void LabeledTextField::move(sf::Vector2f offset) {
    setPosition(getPosition() + offset);
}

sf::Vector2f LabeledTextField::getPosition() const {
    return _position;
}

void LabeledTextField::handleEvent(const sf::Event& event) {
    _textField->handleEvent(event);
}

void LabeledTextField::draw(sf::RenderWindow& window) {
    window.draw(*_labelBox);
    _textField->draw(window);
    window.draw(*_labelText);
}

LabeledTextField::~LabeledTextField() {
    delete _textField;
    delete _labelText;
    delete _labelBox;
}