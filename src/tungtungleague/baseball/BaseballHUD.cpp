#include "tungtungleague/baseball/BaseballHUD.h"

#include <string>

namespace tungtung {

BaseballHUD::BaseballHUD(sf::RenderWindow& window, sf::Font& font)
    : window_(window),
      font_(font),
      inningOutsText_(font_, "", 20),
      scoreText_(font_, "", 20) {
    inningOutsText_.setFillColor(sf::Color::White);
    scoreText_.setFillColor(sf::Color::White);

    refreshTexts();
}

void BaseballHUD::setInning(int inning) {
    inning_ = inning;
    refreshTexts();
}

void BaseballHUD::setOuts(int outs) {
    outs_ = outs;
    refreshTexts();
}

void BaseballHUD::setScores(int teamA, int teamB) {
    teamAScore_ = teamA;
    teamBScore_ = teamB;
    refreshTexts();
}

void BaseballHUD::draw() {
    layout();

    // Top-left area intentionally left empty for PauseMenu icon.
    window_.draw(inningOutsText_);
    window_.draw(scoreText_);
}

void BaseballHUD::refreshTexts() {
    inningOutsText_.setString("INN: " + std::to_string(inning_) + "  |  OUTS: " + std::to_string(outs_));
    scoreText_.setString("TEAM A: " + std::to_string(teamAScore_) + "  |  TEAM B: " + std::to_string(teamBScore_));
}

void BaseballHUD::layout() {
    const float windowWidth = static_cast<float>(window_.getSize().x);

    // Center HUD text at top, with margin for readability.
    const auto innBounds = inningOutsText_.getLocalBounds();
    inningOutsText_.setPosition({
        (windowWidth - innBounds.size.x) * 0.5f - innBounds.position.x,
        16.f - innBounds.position.y
    });

    // Keep right text near right edge.
    const auto scoreBounds = scoreText_.getLocalBounds();
    scoreText_.setPosition({
        windowWidth - scoreBounds.size.x - 24.f - scoreBounds.position.x,
        16.f - scoreBounds.position.y
    });
}

} // namespace tungtung
