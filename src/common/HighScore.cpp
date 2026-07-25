#include "common/HighScore.h"

HighScore::HighScore(corezone::GameDataManager& gameData)
    : Score(), gameData_(gameData) {
    load();
}

void HighScore::load() {
    int saved = 0;
    gameData_.getHighScore(saved);
    score_ = saved;
}

void HighScore::save() {
    gameData_.saveHighScore(score_);
}

bool HighScore::isNewHighScore(int currentScore) const {
    return currentScore > score_;
}

void HighScore::set(int value) {
    if (value > score_) {
        score_ = value;
        save();
    }
}
