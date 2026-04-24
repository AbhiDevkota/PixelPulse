#include "tungtungleague/cricket/CricketField.h"

namespace tungtung {

CricketField::CricketField(sf::RenderWindow& window)
    : window_(window),
      oval_(360.f),
      creaseLines_(sf::PrimitiveType::Lines, 4) {
    buildGeometry();
}

void CricketField::draw() {
    window_.draw(outfield_);
    window_.draw(oval_);
    window_.draw(pitch_);

    for (const auto& stump : verticalStumps_) {
        window_.draw(stump);
    }

    for (const auto& bail : bails_) {
        window_.draw(bail);
    }

    window_.draw(creaseLines_);
}

sf::Vector2f CricketField::getBatterPosition() const {
    return batterPosition_;
}

sf::Vector2f CricketField::getBowlerStartPosition() const {
    return bowlerStartPosition_;
}

sf::Vector2f CricketField::getBowlerCreasePosition() const {
    return bowlerCreasePosition_;
}

sf::Vector2f CricketField::getKeeperPosition() const {
    return keeperPosition_;
}

sf::Vector2f CricketField::getStumpCenter(int end) const {
    return (end == 0) ? batterStumpCenter_ : bowlerStumpCenter_;
}

void CricketField::buildGeometry() {
    const sf::Vector2u size = window_.getSize();
    const float width = static_cast<float>(size.x);
    const float height = static_cast<float>(size.y);

    // Plain darker green outfield.
    outfield_.setSize({ width, height });
    outfield_.setPosition({ 0.f, 0.f });
    outfield_.setFillColor(sf::Color(20, 100, 20));

    // Large oval field in slightly lighter green.
    oval_.setFillColor(sf::Color(35, 130, 35));
    oval_.setOrigin({ oval_.getRadius(), oval_.getRadius() });
    oval_.setScale({ 1.9f, 1.25f });
    oval_.setPosition({ width * 0.5f, height * 0.56f });

    // Center pitch.
    const sf::Vector2f pitchSize(60.f, 280.f);
    pitch_.setSize(pitchSize);
    pitch_.setOrigin({ pitchSize.x * 0.5f, pitchSize.y * 0.5f });
    pitch_.setPosition({ width * 0.5f, height * 0.56f });
    pitch_.setFillColor(sf::Color(210, 180, 140));

    // Stump centers at both pitch ends.
    batterStumpCenter_ = { pitch_.getPosition().x, pitch_.getPosition().y + (pitchSize.y * 0.5f) - 10.f };
    bowlerStumpCenter_ = { pitch_.getPosition().x, pitch_.getPosition().y - (pitchSize.y * 0.5f) + 10.f };

    const sf::Vector2f stumpSize(6.f, 40.f);
    const float spacing = 10.f;
    const sf::Color stumpColor(255, 215, 0);

    // Batter-end vertical stumps (0..2)
    for (int i = 0; i < 3; ++i) {
        auto& stump = verticalStumps_[static_cast<std::size_t>(i)];
        stump.setSize(stumpSize);
        stump.setOrigin({ stumpSize.x * 0.5f, stumpSize.y });
        stump.setFillColor(stumpColor);
        stump.setPosition({ batterStumpCenter_.x + (static_cast<float>(i) - 1.f) * spacing, batterStumpCenter_.y });
    }

    // Bowler-end vertical stumps (3..5)
    for (int i = 0; i < 3; ++i) {
        auto& stump = verticalStumps_[static_cast<std::size_t>(3 + i)];
        stump.setSize(stumpSize);
        stump.setOrigin({ stumpSize.x * 0.5f, stumpSize.y });
        stump.setFillColor(stumpColor);
        stump.setPosition({ bowlerStumpCenter_.x + (static_cast<float>(i) - 1.f) * spacing, bowlerStumpCenter_.y });
    }

    // Bails: 2 at each end.
    const sf::Vector2f bailSize(12.f, 4.f);
    for (int i = 0; i < 4; ++i) {
        bails_[static_cast<std::size_t>(i)].setSize(bailSize);
        bails_[static_cast<std::size_t>(i)].setFillColor(stumpColor);
    }

    bails_[0].setPosition({ batterStumpCenter_.x - 11.f, batterStumpCenter_.y - stumpSize.y - 2.f });
    bails_[1].setPosition({ batterStumpCenter_.x + 1.f, batterStumpCenter_.y - stumpSize.y - 2.f });
    bails_[2].setPosition({ bowlerStumpCenter_.x - 11.f, bowlerStumpCenter_.y - stumpSize.y - 2.f });
    bails_[3].setPosition({ bowlerStumpCenter_.x + 1.f, bowlerStumpCenter_.y - stumpSize.y - 2.f });

    // Crease lines (white horizontal).
    const float creaseHalfWidth = 46.f;
    const float batterCreaseY = batterStumpCenter_.y + 6.f;
    const float bowlerCreaseY = bowlerStumpCenter_.y + 6.f;

    creaseLines_[0].position = { pitch_.getPosition().x - creaseHalfWidth, batterCreaseY };
    creaseLines_[1].position = { pitch_.getPosition().x + creaseHalfWidth, batterCreaseY };
    creaseLines_[2].position = { pitch_.getPosition().x - creaseHalfWidth, bowlerCreaseY };
    creaseLines_[3].position = { pitch_.getPosition().x + creaseHalfWidth, bowlerCreaseY };

    for (std::size_t i = 0; i < 4; ++i) {
        creaseLines_[i].color = sf::Color::White;
    }

    // Gameplay anchor positions.
    batterPosition_ = { batterStumpCenter_.x + 20.f, batterStumpCenter_.y + 8.f };
    bowlerCreasePosition_ = { bowlerStumpCenter_.x, bowlerStumpCenter_.y - 8.f };
    bowlerStartPosition_ = { bowlerStumpCenter_.x, bowlerStumpCenter_.y - 120.f };
    keeperPosition_ = { batterStumpCenter_.x, batterStumpCenter_.y + 60.f };
}

} // namespace tungtung
