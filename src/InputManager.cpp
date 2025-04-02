#include "InputManager.h"


bool InputManager::IsSpriteClicked(sf::Sprite object, sf::Mouse::Button button, sf::RenderWindow &window) {
    if (sf::Mouse::isButtonPressed(button)) {
        sf::IntRect tempRect(object.getPosition().x, object.getPosition().y, object.getGlobalBounds().width, object.getGlobalBounds().height);
        if (tempRect.contains(sf::Mouse::getPosition(window))) {
            return true;
        }
    }
    return false;
}

bool InputManager::IsSpriteClickedAccurate(sf::Sprite object, sf::Mouse::Button button, sf::RenderWindow &window) {
    if (sf::Mouse::isButtonPressed(button)) {

        sf::Vector2f mouseWorldPos = GetMousePosition(window);

        // Zamiana pozycji myszy na współrzędne tekstury sprite'a
        sf::FloatRect bounds = object.getGlobalBounds();
        sf::Vector2f spritePosition = object.getPosition();

        // Sprawdzenie, czy kliknięcie znajduje się w globalnych granicach sprite'a
        if (bounds.contains(mouseWorldPos)) {
            // Pobranie tekstury i pikseli
            const sf::Texture* texture = object.getTexture();
            if (texture) {
                // Pobranie obrazu tekstury (musi być załadowana wcześniej jako `sf::Image`)
                sf::Image image = texture->copyToImage();

                // Pozycja piksela w teksturze (z uwzględnieniem skali i przesunięcia)
                sf::Vector2f localPos = mouseWorldPos - spritePosition;
                sf::Vector2f scaledPos(localPos.x / object.getScale().x, localPos.y / object.getScale().y);

                // Upewnienie się, że piksel jest w granicach tekstury
                if (scaledPos.x >= 0 && scaledPos.y >= 0 &&
                    scaledPos.x < image.getSize().x && scaledPos.y < image.getSize().y) {

                    // Pobranie piksela w odpowiednich współrzędnych
                    sf::Color pixelColor = image.getPixel(static_cast<unsigned int>(scaledPos.x),
                                                          static_cast<unsigned int>(scaledPos.y));

                    // Sprawdzenie, czy piksel nie jest przezroczysty
                    if (pixelColor.a != 0) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool InputManager::IsSpriteHover(sf::Sprite object, sf::Mouse::Button button, sf::RenderWindow &window) {
    sf::IntRect tempRect(object.getPosition().x, object.getPosition().y, object.getGlobalBounds().width, object.getGlobalBounds().height);
    if (tempRect.contains(sf::Mouse::getPosition(window))) {
        return true;
    }
    return false;
}
bool InputManager::IsSpriteHoverAccurate(sf::Sprite object, sf::Mouse::Button button, sf::RenderWindow &window) {
   
    sf::Vector2f mouseWorldPos = GetMousePosition(window);

    // Zamiana pozycji myszy na współrzędne tekstury sprite'a
    sf::FloatRect bounds = object.getGlobalBounds();
    sf::Vector2f spritePosition = object.getPosition();

    // Sprawdzenie, czy kliknięcie znajduje się w globalnych granicach sprite'a
    if (bounds.contains(mouseWorldPos)) {
        // Pobranie tekstury i pikseli
        const sf::Texture* texture = object.getTexture();
        if (texture) {
            // Pobranie obrazu tekstury (musi być załadowana wcześniej jako `sf::Image`)
            sf::Image image = texture->copyToImage();

            // Pozycja piksela w teksturze (z uwzględnieniem skali i przesunięcia)
            sf::Vector2f localPos = mouseWorldPos - spritePosition;
            sf::Vector2f scaledPos(localPos.x / object.getScale().x, localPos.y / object.getScale().y);

            // Upewnienie się, że piksel jest w granicach tekstury
            if (scaledPos.x >= 0 && scaledPos.y >= 0 &&
                scaledPos.x < image.getSize().x && scaledPos.y < image.getSize().y) {

                // Pobranie piksela w odpowiednich współrzędnych
                sf::Color pixelColor = image.getPixel(static_cast<unsigned int>(scaledPos.x),
                                                        static_cast<unsigned int>(scaledPos.y));

                // Sprawdzenie, czy piksel nie jest przezroczysty
                if (pixelColor.a != 0) {
                    return true;
                }
            }
        }
    }

    return false;
}


bool InputManager::IsTextClicked(sf::Text text, sf::Mouse::Button button, sf::RenderWindow &window) {
    if (sf::Mouse::isButtonPressed(button)) {
        sf::IntRect tempRect(text.getPosition().x, text.getPosition().y, text.getGlobalBounds().width, text.getGlobalBounds().height);
        if (tempRect.contains(sf::Mouse::getPosition(window))) {
            return true;
        }
    }
    return false;
}

sf::Vector2f InputManager::GetMousePosition(sf::RenderWindow &window) {
    sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
    return window.mapPixelToCoords(mousePosition);
}