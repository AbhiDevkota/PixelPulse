#include "gameaudio.h"

GameAudio::GameAudio()
    : jumpSound(jumpSoundBuffer)
{
}

bool GameAudio::load() {
    // Background music
    if (!music.openFromFile("audios/Flappy/flappy.mp3")) {
        return false;
    }

    music.setVolume(50.f);
    music.setLooping(true);
    music.play();

    // Jump sound
    if (!jumpSoundBuffer.loadFromFile("audios/Flappy/jumpsound.mp3")) {
        return false;
    }

    jumpSound.setVolume(50.f);
    jumpSound.setLooping(true);
    jumpSound.play();

    return true;
}

void GameAudio::playJump() {
    jumpSound.play();
}