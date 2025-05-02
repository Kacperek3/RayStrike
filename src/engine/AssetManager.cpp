#include "AssetManager.h"

void AssetManager::LoadTexture(std::string name, std::string fileName) {
    // Sprawdzenie, czy tekstura już istnieje
    if (this->_textures.find(name) != this->_textures.end()) {
        std::cout << "Tekstura '" << name << "' już istnieje. Nie dodajemy ponownie." << std::endl;
        return; // Wyjdź z funkcji, jeśli tekstura już istnieje
    }

    sf::Texture texture;
    if (texture.loadFromFile(fileName)) {
        this->_textures[name] = texture;
        std::cout << "Załadowano teksturę '" << name << "' z pliku '" << fileName << "'." << std::endl;
    } else {
        std::cout << "Nie udało się załadować tekstury z pliku '" << fileName << "'." << std::endl;
    }
}


sf::Texture &AssetManager::GetTexture(std::string name) {
    return this->_textures.at(name);
}

void AssetManager::LoadFont(std::string name, std::string fileName) {
    sf::Font font;
    if (font.loadFromFile(fileName)) {
        this->_fonts[name] = font;
    }
}

sf::Font &AssetManager::GetFont(std::string name) {
    return this->_fonts.at(name);
}

void AssetManager::clearAssets() {
    _textures.clear();
    _fonts.clear();
}


AssetManager::~AssetManager() {
   std::cout << "AssetManager destructor" << std::endl;
}