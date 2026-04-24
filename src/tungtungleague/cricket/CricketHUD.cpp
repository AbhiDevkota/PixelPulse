#include "tungtungleague/cricket/CricketHUD.h"

#include <string>

namespace tungtung {

CricketHUD::CricketHUD(sf::RenderWindow& window, sf::Font& font)
    : window_(window),
      font_(font),
      scoreOversText_(font_, "", 20),
      targetText_(font_, "", 20) {
    scoreOversText_.setFillColor(sf::Color::White);
    targetText_.setFillColor(sf::Color::White);

    refreshTexts();
}

void CricketHUD::setScore(int runs, int wickets) {
    runs_ = runs;
    wickets_ = wickets;
    refreshTexts();
}

void CricketHUD::setBalls(int totalBalls) {
    balls_ = totalBalls;
    refreshTexts();
}

void CricketHUD::setTarget(int target) {
    target_ = target;
    refreshTexts();
}

void CricketHUD::draw() {
    layout();

    // Left side intentionally left free for PauseMenu icon.
    window_.draw(scoreOversText_);

    if (target_ >= 0) {
        window_.draw(targetText_);
    }
}

void CricketHUD::refreshTexts() {
    const int overs = balls_ / 6;
    const int ballsInOver = balls_ % 6;

    scoreOversText_.setString(
        "SCORE: " + std::to_string(runs_) + "/" + std::to_string(wickets_) +
        "  |  OVR: " + std::to_string(overs) + "." + std::to_string(ballsInOver)
    );

    if (target_ >= 0) {
        targetText_.setString("TARGET: " + std::to_string(target_));
    }
    else {
        targetText_.setString("TARGET: --");
    }
}

void CricketHUD::layout() {
    const float windowWidth = static_cast<float>(window_.getSize().x);

    const auto scoreBounds = scoreOversText_.getLocalBounds();
    scoreOversText_.setPosition({
        (windowWidth - scoreBounds.size.x) * 0.5f - scoreBounds.position.x,
        16.f - scoreBounds.position.y
    });

    const auto targetBounds = targetText_.getLocalBounds();
    targetText_.setPosition({
        windowWidth - targetBounds.size.x - 24.f - targetBounds.position.x,
        16.f - targetBounds.position.y
    });
}

} // namespace tungtung
