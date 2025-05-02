#pragma once

#include <SFML/Audio.hpp>
#include <map>
#include <string>
#include <iostream>

class SoundManager {
public:
    SoundManager() {}
    ~SoundManager();

    void LoadSoundBuffer(const std::string& name, const std::string& fileName);
    sf::SoundBuffer& GetSoundBuffer(const std::string& name);

    sf::Sound CreateSound(const std::string& bufferName);

    //void LoadMusic(const std::string& name, const std::string& fileName);
    //sf::Music& GetMusic(const std::string& name);

    void ClearSounds();

private:
    std::map<std::string, sf::SoundBuffer> _soundBuffers;
    //std::map<std::string, sf::Music> _musicTracks;
};
