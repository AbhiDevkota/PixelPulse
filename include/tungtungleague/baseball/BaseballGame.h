#ifndef TUNGTUNG_BASEBALLGAME_H
#define TUNGTUNG_BASEBALLGAME_H

#include <SFML/Graphics.hpp>
#include <array>
#include <functional>
#include <random>
#include <string>
#include <vector>

#include "tungtungleague/baseball/BaseballField.h"
#include "tungtungleague/baseball/BaseballHUD.h"
#include "tungtungleague/common/Ball.h"
#include "tungtungleague/common/PauseMenu.h"
#include "tungtungleague/common/Player.h"

namespace tungtung {

class BaseballGame {
public:
    BaseballGame(sf::RenderWindow& window, sf::Font& font, const sf::Texture& texture);

    void update(float dt);
    void draw();
    void handleInput(const sf::Event& event);
    void handleMouseMove(sf::Vector2f pos);
    void handleMouseClick(sf::Vector2f pos);
    void reset();

    std::function<void()> onExit;

private:
    enum class Outcome {
        StrikeOut,
        Single,
        Double,
        HomeRun,
        CaughtOut
    };

    Outcome rollOutcome();
    void performAtBat();
    void applyOutcome(Outcome outcome);
    void advanceRunners(int bases);
    void handleThreeOuts();
    void updateHud();
    void setupPlayers(const sf::Texture& texture);

    sf::RenderWindow& window_;
    sf::Font& font_;

    BaseballField field_;
    BaseballHUD hud_;
    PauseMenu pauseMenu_;
    Ball ball_;

    std::vector<Player> players_;

    int inning_ = 1;
    bool teamABatting_ = true;
    int outs_ = 0;
    int teamAScore_ = 0;
    int teamBScore_ = 0;

    std::array<bool, 3> bases_{ false, false, false }; // 1st, 2nd, 3rd

    bool gameOver_ = false;
    std::string winnerText_;

    std::string outcomeText_;
    sf::Text outcomeDrawText_;
    float outcomeTimer_ = 0.f;

    sf::RectangleShape winnerOverlay_;
    sf::Text winnerDrawText_;

    std::mt19937 rng_;
    std::discrete_distribution<int> outcomeDist_;
};

} // namespace tungtung

#endif // TUNGTUNG_BASEBALLGAME_H
