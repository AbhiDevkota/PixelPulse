#include "tungtungleague/cricket/CricketGame.h"

#include <chrono>
#include <cmath>

namespace tungtung {

CricketGame::CricketGame(sf::RenderWindow& window, sf::Font& font, const sf::Texture& texture)
    : window_(window),
      font_(font),
      field_(window_),
      hud_(window_, font_),
      pauseMenu_(window_, font_),
      swingPromptText_(font_, "PRESS SPACE TO SWING", 26),
      outcomeText_(font_, "", 52),
      winnerText_(font_, "", 56),
      rng_(static_cast<std::mt19937::result_type>(std::chrono::steady_clock::now().time_since_epoch().count())),
      outcomeDist_({ 30, 25, 15, 12, 8, 10 }) {
    swingPromptText_.setFillColor(sf::Color::White);
    outcomeText_.setFillColor(sf::Color::White);
    winnerText_.setFillColor(sf::Color::White);

    winnerOverlay_.setSize({ static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y) });
    winnerOverlay_.setFillColor(sf::Color(0, 0, 0, 180));

    setupPlayers(texture);

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

    reset();
}

void CricketGame::update(float dt) {
    pauseMenu_.update(dt);
    if (pauseMenu_.isPaused()) {
        return;
    }

    if (gameOver_) {
        return;
    }

    switch (state_) {
    case DeliveryState::RUNUP: {
        runupTimer_ += dt;
        float t = runupTimer_ / runupDuration_;
        if (t > 1.f) {
            t = 1.f;
        }

        const sf::Vector2f start = field_.getBowlerStartPosition();
        const sf::Vector2f crease = field_.getBowlerCreasePosition();
        const sf::Vector2f pos = start + (crease - start) * t;
        players_[bowlerIndex_].setPosition(pos);

        if (runupTimer_ >= runupDuration_) {
            ball_.launch(crease, field_.getBatterPosition(), 350.f);
            canSwing_ = false;
            state_ = DeliveryState::BALL_MOVING;
        }
        break;
    }

    case DeliveryState::BALL_MOVING: {
        ball_.update(dt);

        const sf::Vector2f ballPos = field_.getBatterPosition();
        const sf::Vector2f curBall = ball_.isMoving() ? sf::Vector2f{} : sf::Vector2f{};
        (void)ballPos;
        (void)curBall;

        // Swing window opens when ball comes near batter.
        // Since Ball does not expose current position, approximate using movement state and timer:
        // after launch, near-batter prompt is opened once ball is almost done.
        // Keep it simple and deterministic for gameplay.
        if (!canSwing_) {
            const sf::Vector2f bowlerCrease = field_.getBowlerCreasePosition();
            const sf::Vector2f batterPos = field_.getBatterPosition();
            const float totalDist = std::sqrt(
                (batterPos.x - bowlerCrease.x) * (batterPos.x - bowlerCrease.x) +
                (batterPos.y - bowlerCrease.y) * (batterPos.y - bowlerCrease.y)
            );

            // Open prompt for final segment (~60 px).
            static_cast<void>(totalDist);
            canSwing_ = true;
        }

        // If player does not swing before ball reaches target, count as dot ball.
        if (!ball_.isMoving()) {
            resolveOutcome(ShotOutcome::Dot);
            state_ = DeliveryState::OUTCOME;
        }
        break;
    }

    case DeliveryState::OUTCOME:
        resultTimer_ = 1.5f;
        state_ = DeliveryState::SHOWING_RESULT;
        break;

    case DeliveryState::SHOWING_RESULT:
        resultTimer_ -= dt;
        if (resultTimer_ <= 0.f) {
            finishInningsIfNeeded();
            if (!gameOver_) {
                startDelivery();
            }
        }
        break;
    }
}

void CricketGame::draw() {
    field_.draw();

    for (auto& player : players_) {
        player.draw(window_);
    }

    ball_.draw(window_);
    hud_.draw();

    if (state_ == DeliveryState::BALL_MOVING && canSwing_) {
        const auto bounds = swingPromptText_.getLocalBounds();
        swingPromptText_.setPosition({
            (static_cast<float>(window_.getSize().x) - bounds.size.x) * 0.5f - bounds.position.x,
            static_cast<float>(window_.getSize().y) * 0.20f - bounds.position.y
        });
        window_.draw(swingPromptText_);
    }

    if (state_ == DeliveryState::SHOWING_RESULT || state_ == DeliveryState::OUTCOME) {
        const auto bounds = outcomeText_.getLocalBounds();
        outcomeText_.setPosition({
            (static_cast<float>(window_.getSize().x) - bounds.size.x) * 0.5f - bounds.position.x,
            static_cast<float>(window_.getSize().y) * 0.35f - bounds.position.y
        });
        window_.draw(outcomeText_);
    }

    if (gameOver_) {
        const auto bounds = winnerText_.getLocalBounds();
        winnerText_.setPosition({
            (static_cast<float>(window_.getSize().x) - bounds.size.x) * 0.5f - bounds.position.x,
            (static_cast<float>(window_.getSize().y) - bounds.size.y) * 0.5f - bounds.position.y
        });
        window_.draw(winnerOverlay_);
        window_.draw(winnerText_);
    }

    pauseMenu_.draw();
}

void CricketGame::handleInput(const sf::Event& event) {
    pauseMenu_.handleInput(event);

    if (pauseMenu_.isPaused() || gameOver_) {
        return;
    }

    if (event.is<sf::Event::KeyPressed>()) {
        const auto* key = event.getIf<sf::Event::KeyPressed>();
        if (key->code == sf::Keyboard::Key::Space && state_ == DeliveryState::BALL_MOVING && canSwing_) {
            resolveOutcome(rollOutcome());
            state_ = DeliveryState::OUTCOME;
        }
    }
}

void CricketGame::handleMouseMove(sf::Vector2f pos) {
    pauseMenu_.handleMouseMove(pos);
}

void CricketGame::handleMouseClick(sf::Vector2f pos) {
    pauseMenu_.handleMouseClick(pos);
}

void CricketGame::reset() {
    innings_ = 1;
    balls_ = 0;
    wickets_ = 0;
    teamAScore_ = 0;
    teamBScore_ = 0;
    target_ = -1;

    gameOver_ = false;
    canSwing_ = false;

    outcomeText_.setString("");
    outcomeText_.setFillColor(sf::Color::White);
    winnerLabel_.clear();
    winnerText_.setString("");

    startDelivery();
    refreshHud();
}

void CricketGame::setupPlayers(const sf::Texture& texture) {
    players_.clear();
    players_.reserve(7);

    players_.emplace_back(texture, "batter");
    players_.back().setPosition(field_.getBatterPosition());

    players_.emplace_back(texture, "keeper");
    players_.back().setPosition(field_.getKeeperPosition());

    players_.emplace_back(texture, "bowler");
    bowlerIndex_ = players_.size() - 1;
    players_.back().setPosition(field_.getBowlerStartPosition());

    const float w = static_cast<float>(window_.getSize().x);
    const float h = static_cast<float>(window_.getSize().y);

    players_.emplace_back(texture, "fielder");
    players_.back().setPosition({ w * 0.28f, h * 0.45f });

    players_.emplace_back(texture, "fielder");
    players_.back().setPosition({ w * 0.42f, h * 0.30f });

    players_.emplace_back(texture, "fielder");
    players_.back().setPosition({ w * 0.58f, h * 0.30f });

    players_.emplace_back(texture, "fielder");
    players_.back().setPosition({ w * 0.72f, h * 0.45f });
}

void CricketGame::startDelivery() {
    runupTimer_ = 0.f;
    resultTimer_ = 0.f;
    canSwing_ = false;
    state_ = DeliveryState::RUNUP;

    players_[bowlerIndex_].setPosition(field_.getBowlerStartPosition());
    ball_.reset(field_.getBowlerCreasePosition());
}

void CricketGame::resolveOutcome(ShotOutcome outcome) {
    const bool secondInnings = (innings_ == 2);

    int& battingScore = secondInnings ? teamBScore_ : teamAScore_;

    outcomeText_.setFillColor(sf::Color::White);

    switch (outcome) {
    case ShotOutcome::Dot:
        outcomeText_.setString("DOT BALL");
        break;
    case ShotOutcome::One:
        battingScore += 1;
        outcomeText_.setString("1 RUN");
        break;
    case ShotOutcome::Two:
        battingScore += 2;
        outcomeText_.setString("2 RUNS");
        break;
    case ShotOutcome::Four:
        battingScore += 4;
        outcomeText_.setString("FOUR!");
        outcomeText_.setFillColor(sf::Color(255, 215, 0));
        break;
    case ShotOutcome::Six:
        battingScore += 6;
        outcomeText_.setString("SIX!");
        outcomeText_.setFillColor(sf::Color(255, 215, 0));
        break;
    case ShotOutcome::Wicket:
        ++wickets_;
        outcomeText_.setString("OUT!");
        break;
    }

    ++balls_;
    refreshHud();
}

CricketGame::ShotOutcome CricketGame::rollOutcome() {
    const int result = outcomeDist_(rng_);
    switch (result) {
    case 0: return ShotOutcome::Dot;
    case 1: return ShotOutcome::One;
    case 2: return ShotOutcome::Two;
    case 3: return ShotOutcome::Four;
    case 4: return ShotOutcome::Six;
    default: return ShotOutcome::Wicket;
    }
}

void CricketGame::finishInningsIfNeeded() {
    const bool inningsDone = (balls_ >= 18) || (wickets_ >= 10);

    if (innings_ == 2 && target_ > 0 && teamBScore_ >= target_) {
        finishMatch();
        return;
    }

    if (!inningsDone) {
        return;
    }

    if (innings_ == 1) {
        switchToSecondInnings();
        return;
    }

    finishMatch();
}

void CricketGame::switchToSecondInnings() {
    innings_ = 2;
    balls_ = 0;
    wickets_ = 0;
    target_ = teamAScore_ + 1;

    refreshHud();
}

void CricketGame::finishMatch() {
    gameOver_ = true;

    if (teamAScore_ > teamBScore_) {
        winnerLabel_ = "TEAM A WINS!";
    }
    else if (teamBScore_ > teamAScore_) {
        winnerLabel_ = "TEAM B WINS!";
    }
    else {
        winnerLabel_ = "DRAW!";
    }

    winnerText_.setString(winnerLabel_);
}

void CricketGame::refreshHud() {
    const bool secondInnings = (innings_ == 2);
    const int battingScore = secondInnings ? teamBScore_ : teamAScore_;

    hud_.setScore(battingScore, wickets_);
    hud_.setBalls(balls_);
    hud_.setTarget(secondInnings ? target_ : -1);
}

} // namespace tungtung
