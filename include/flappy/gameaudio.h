#pragma once

#include <SFML/Audio.hpp>

class GameAudio {
public:
    sf::Music music;
    sf::SoundBuffer jumpSoundBuffer;
    sf::Sound jumpSound;
    sf::SoundBuffer highScoreBuffer;
    sf::Sound highScoreSound{ highScoreBuffer };
    void playHighScore();

    GameAudio();

    bool load();
    void playJump();
};