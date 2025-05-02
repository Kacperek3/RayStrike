#include "SoundManager.h"

SoundManager::~SoundManager() {
    ClearSounds();
}

void SoundManager::LoadSoundBuffer(const std::string& name, const std::string& fileName) {
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(fileName)) {
        std::cerr << "Failed to load sound buffer from file: " << fileName << std::endl;
        return;
    }
    _soundBuffers[name] = std::move(buffer);
}

sf::SoundBuffer& SoundManager::GetSoundBuffer(const std::string& name) {
    return _soundBuffers.at(name);
}

sf::Sound SoundManager::CreateSound(const std::string& bufferName) {
    sf::Sound sound;
    sound.setBuffer(GetSoundBuffer(bufferName));
    return sound;
}

// void SoundManager::LoadMusic(const std::string& name, const std::string& fileName) {
//     sf::Music music;
//     if (!music.openFromFile(fileName)) {
//         std::cerr << "Failed to load music from file: " << fileName << std::endl;
//         return;
//     }
//     _musicTracks[name] = std::move(music);
// }

// sf::Music& SoundManager::GetMusic(const std::string& name) {
//     return _musicTracks.at(name);
// }

void SoundManager::ClearSounds() {
    _soundBuffers.clear();
    //_musicTracks.clear();
}
