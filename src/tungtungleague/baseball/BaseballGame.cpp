#include "tungtungleague/baseball/BaseballGame.h"

#include <chrono>

namespace tungtung {

BaseballGame::BaseballGame(sf::RenderWindow& window, sf::Font& font, const sf::Texture& texture)
    : window_(window),
      font_(font),
      field_(window_),
      hud_(window_, font_),
      pauseMenu_(window_, font_),
      outcomeDrawText_(font_, "", 42),
      winnerDrawText_(font_, "", 56),
      rng_(static_cast<std::mt19937::result_type>(std::chrono::steady_clock::now().time_since_epoch().count())),
      outcomeDist_({ 40, 25, 15, 10, 10 }) {
    outcomeDrawText_.setFillColor(sf::Color::White);

    winnerOverlay_.setSize({ static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y) });
    winnerOverlay_.setFillColor(sf::Color(0, 0, 0, 180));

    winnerDrawText_.setFillColor(sf::Color::White);

    setupPlayers(texture);
    reset();

    pauseMenu_.onResume = [this]() {
        pauseMenu_.close();
    };

    pauseMenu_.onPlayAgain = [this]() {
        reset();
    };

    pauseMenu_.onExit = [this]() {
        if (onExit) {
            onExit();
        }
    };
}

void BaseballGame::update(float dt) {
    pauseMenu_.update(dt);

    if (pauseMenu_.isPaused()) {
        return;
    }

    ball_.update(dt);

    if (outcomeTimer_ > 0.f) {
        outcomeTimer_ -= dt;
        if (outcomeTimer_ < 0.f) {
            outcomeTimer_ = 0.f;
        }
    }
}

void BaseballGame::draw() {
    field_.draw();

    for (auto& player : players_) {
        player.draw(window_);
    }

    ball_.draw(window_);
    hud_.draw();

    if (outcomeTimer_ > 0.f) {
        const auto bounds = outcomeDrawText_.getLocalBounds();
        outcomeDrawText_.setPosition({
            (static_cast<float>(window_.getSize().x) - bounds.size.x) * 0.5f - bounds.position.x,
            static_cast<float>(window_.getSize().y) * 0.24f - bounds.position.y
        });
        window_.draw(outcomeDrawText_);
    }

    if (gameOver_) {
        const auto bounds = winnerDrawText_.getLocalBounds();
        winnerDrawText_.setPosition({
            (static_cast<float>(window_.getSize().x) - bounds.size.x) * 0.5f - bounds.position.x,
            (static_cast<float>(window_.getSize().y) - bounds.size.y) * 0.5f - bounds.position.y
        });
        window_.draw(winnerOverlay_);
        window_.draw(winnerDrawText_);
    }

    pauseMenu_.draw();
}

void BaseballGame::handleInput(const sf::Event& event) {
    pauseMenu_.handleInput(event);

    if (pauseMenu_.isPaused() || gameOver_) {
        return;
    }

    if (event.is<sf::Event::KeyPressed>()) {
        const auto* key = event.getIf<sf::Event::KeyPressed>();
        if (key->code == sf::Keyboard::Key::Space && outcomeTimer_ <= 0.f) {
            performAtBat();
        }
    }
}

void BaseballGame::handleMouseMove(sf::Vector2f pos) {
    pauseMenu_.handleMouseMove(pos);
}

void BaseballGame::handleMouseClick(sf::Vector2f pos) {
    pauseMenu_.handleMouseClick(pos);
}

void BaseballGame::reset() {
    inning_ = 1;
    teamABatting_ = true;
    outs_ = 0;
    teamAScore_ = 0;
    teamBScore_ = 0;
    bases_ = { false, false, false };

    gameOver_ = false;
    winnerText_.clear();
    winnerDrawText_.setString("");

    outcomeText_.clear();
    outcomeDrawText_.setString("");
    outcomeDrawText_.setFillColor(sf::Color::White);
    outcomeTimer_ = 0.f;

    ball_.reset(field_.getPitcherPosition());
    updateHud();
}

BaseballGame::Outcome BaseballGame::rollOutcome() {
    const int result = outcomeDist_(rng_);
    switch (result) {
    case 0: return Outcome::StrikeOut;
    case 1: return Outcome::Single;
    case 2: return Outcome::Double;
    case 3: return Outcome::HomeRun;
    default: return Outcome::CaughtOut;
    }
}

void BaseballGame::performAtBat() {
    ball_.launch(field_.getPitcherPosition(), field_.getBatterPosition(), 400.f);

    const Outcome outcome = rollOutcome();
    applyOutcome(outcome);

    outcomeDrawText_.setString(outcomeText_);
    outcomeTimer_ = 1.5f;

    updateHud();
}

void BaseballGame::applyOutcome(Outcome outcome) {
    switch (outcome) {
    case Outcome::StrikeOut:
        ++outs_;
        outcomeText_ = "STRIKE!";
        break;

    case Outcome::Single:
        advanceRunners(1);
        outcomeText_ = "SINGLE!";
        break;

    case Outcome::Double:
        advanceRunners(2);
        outcomeText_ = "DOUBLE!";
        break;

    case Outcome::HomeRun: {
        int runs = 1;
        for (bool occupied : bases_) {
            if (occupied) {
                ++runs;
            }
        }

        if (teamABatting_) {
            teamAScore_ += runs;
        }
        else {
            teamBScore_ += runs;
        }

        bases_ = { false, false, false };
        outcomeText_ = "HOME RUN!";
        break;
    }

    case Outcome::CaughtOut:
        ++outs_;
        outcomeText_ = "OUT!";
        break;
    }

    if (outs_ >= 3) {
        handleThreeOuts();
    }
}

void BaseballGame::advanceRunners(int basesToAdvance) {
    std::array<bool, 3> nextBases{ false, false, false };
    int runsScored = 0;

    // Move existing runners.
    for (int i = 2; i >= 0; --i) {
        if (!bases_[static_cast<std::size_t>(i)]) {
            continue;
        }

        const int destination = i + basesToAdvance;
        if (destination >= 3) {
            ++runsScored;
        }
        else {
            nextBases[static_cast<std::size_t>(destination)] = true;
        }
    }

    // Place batter.
    const int batterBase = basesToAdvance - 1;
    if (batterBase >= 3) {
        ++runsScored;
    }
    else {
        nextBases[static_cast<std::size_t>(batterBase)] = true;
    }

    bases_ = nextBases;

    if (teamABatting_) {
        teamAScore_ += runsScored;
    }
    else {
        teamBScore_ += runsScored;
    }
}

void BaseballGame::handleThreeOuts() {
    outs_ = 0;
    bases_ = { false, false, false };

    if (teamABatting_) {
        teamABatting_ = false;
        return;
    }

    teamABatting_ = true;
    ++inning_;

    if (inning_ > 3) {
        gameOver_ = true;

        if (teamAScore_ > teamBScore_) {
            winnerText_ = "TEAM A WINS!";
        }
        else if (teamBScore_ > teamAScore_) {
            winnerText_ = "TEAM B WINS!";
        }
        else {
            winnerText_ = "DRAW!";
        }

        winnerDrawText_.setString(winnerText_);
    }
}

void BaseballGame::updateHud() {
    hud_.setInning(inning_ > 3 ? 3 : inning_);
    hud_.setOuts(outs_);
    hud_.setScores(teamAScore_, teamBScore_);
}

void BaseballGame::setupPlayers(const sf::Texture& texture) {
    players_.clear();
    players_.reserve(10);

    // Batter, catcher, pitcher.
    players_.emplace_back(texture, "batter");
    players_.back().setPosition(field_.getBatterPosition());

    players_.emplace_back(texture, "catcher");
    players_.back().setPosition(field_.getCatcherPosition());

    players_.emplace_back(texture, "pitcher");
    players_.back().setPosition(field_.getPitcherPosition());

    // Basemen at first, second, third.
    players_.emplace_back(texture, "first_baseman");
    players_.back().setPosition(field_.getBasePosition(1));

    players_.emplace_back(texture, "second_baseman");
    players_.back().setPosition(field_.getBasePosition(2));

    players_.emplace_back(texture, "third_baseman");
    players_.back().setPosition(field_.getBasePosition(3));

    // Three outfielders (hardcoded positions).
    const float w = static_cast<float>(window_.getSize().x);
    const float h = static_cast<float>(window_.getSize().y);

    players_.emplace_back(texture, "outfielder");
    players_.back().setPosition({ w * 0.34f, h * 0.34f });

    players_.emplace_back(texture, "outfielder");
    players_.back().setPosition({ w * 0.50f, h * 0.28f });

    players_.emplace_back(texture, "outfielder");
    players_.back().setPosition({ w * 0.66f, h * 0.34f });
}

} // namespace tungtung
