#pragma once

#include <SFML/Audio.hpp>

class GameAudio {
public:
    sf::Music music;
    sf::SoundBuffer jumpSoundBuffer;
    sf::Sound jumpSound;

    GameAudio();

    bool load();
    void playJump();
};