#include "flappy/gameaudio.h"

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

    // Jump sound — just load the buffer, don't play it yet
    if (!jumpSoundBuffer.loadFromFile("audios/Flappy/jumpsound.mp3")) {
        return false;
    }

    jumpSound.setVolume(50.f);
    jumpSound.setLooping(false); // a flap sound should play once, not loop

    return true;
}

void GameAudio::playJump() {
    jumpSound.stop();       // stop any tail end still playing
    jumpSound.play();       // play from the start
}