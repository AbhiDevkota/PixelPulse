#ifndef TUNGTUNG_CRICKETGAME_H
#define TUNGTUNG_CRICKETGAME_H

#include <SFML/Graphics.hpp>
#include <array>
#include <functional>
#include <random>
#include <string>
#include <vector>

#include "tungtungleague/common/Ball.h"
#include "tungtungleague/common/PauseMenu.h"
#include "tungtungleague/common/Player.h"
#include "tungtungleague/cricket/CricketField.h"
#include "tungtungleague/cricket/CricketHUD.h"

namespace tungtung {

class CricketGame {
public:
    CricketGame(sf::RenderWindow& window, sf::Font& font, const sf::Texture& texture);

    void update(float dt);
    void draw();
    void handleInput(const sf::Event& event);
    void handleMouseMove(sf::Vector2f pos);
    void handleMouseClick(sf::Vector2f pos);
    void reset();

    std::function<void()> onExit;

private:
    enum class DeliveryState {
        RUNUP,
        BALL_MOVING,
        OUTCOME,
        SHOWING_RESULT
    };

    enum class ShotOutcome {
        Dot,
        One,
        Two,
        Four,
        Six,
        Wicket
    };

    void setupPlayers(const sf::Texture& texture);
    void startDelivery();
    void resolveOutcome(ShotOutcome outcome);
    ShotOutcome rollOutcome();
    void finishInningsIfNeeded();
    void switchToSecondInnings();
    void finishMatch();
    void refreshHud();

    sf::RenderWindow& window_;
    sf::Font& font_;

    CricketField field_;
    CricketHUD hud_;
    PauseMenu pauseMenu_;
    Ball ball_;

    std::vector<Player> players_;
    std::size_t bowlerIndex_ = 0;

    DeliveryState state_ = DeliveryState::RUNUP;

    int innings_ = 1;
    int balls_ = 0;
    int wickets_ = 0;
    int teamAScore_ = 0;
    int teamBScore_ = 0;
    int target_ = -1;

    bool gameOver_ = false;
    bool canSwing_ = false;

    float runupTimer_ = 0.f;
    const float runupDuration_ = 1.5f;
    float resultTimer_ = 0.f;

    sf::Text swingPromptText_;
    sf::Text outcomeText_;
    sf::RectangleShape winnerOverlay_;
    sf::Text winnerText_;

    std::string winnerLabel_;

    std::mt19937 rng_;
    std::discrete_distribution<int> outcomeDist_;
};

} // namespace tungtung

#endif // TUNGTUNG_CRICKETGAME_H
