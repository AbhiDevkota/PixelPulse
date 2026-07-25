#include "flappy/GameAudio.h"

GameAudio::GameAudio()
    : jumpSound(jumpSoundBuffer)
{
}

bool GameAudio::load() {
    // Background music
    if (!music.openFromFile("audios/flappy/flappy.wav")) {
        return false;
    }

    music.setVolume(50.f);
    music.setLooping(true);
    music.play();

    // Jump sound — just load the buffer, don't play it yet
    if (!jumpSoundBuffer.loadFromFile("audios/flappy/jumpsound.wav")) {
        return false;
    }

    jumpSound.setVolume(50.f);
    jumpSound.setLooping(false); // a flap sound should play once, not loop

    // High score sound
    if (!highScoreBuffer.loadFromFile("audios/flappy/highscoresound.wav")) {
        return false;
    }

    highScoreSound.setVolume(300.f);
    highScoreSound.setLooping(false);

    return true;
}

void GameAudio::playJump() {
    jumpSound.stop();       // stop any tail end still playing
    jumpSound.play();       // play from the start
}

void GameAudio::playHighScore() {
    highScoreSound.stop();
    highScoreSound.play();
}