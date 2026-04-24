#include "tungtungleague/baseball/BaseballField.h"

namespace tungtung {

BaseballField::BaseballField(sf::RenderWindow& window)
    : window_(window),
      outfieldArc_(360.f),
      foulLines_(sf::PrimitiveType::Lines, 4) {
    buildGeometry();
}

void BaseballField::draw() {
    window_.draw(background_);
    window_.draw(outfieldArc_);
    window_.draw(diamond_);

    for (const auto& base : baseSquares_) {
        window_.draw(base);
    }

    window_.draw(homePlate_);
    window_.draw(pitcherMound_);
    window_.draw(foulLines_);
}

sf::Vector2f BaseballField::getBasePosition(int base) const {
    if (base < 0 || base > 3) {
        return basePositions_[0];
    }
    return basePositions_[static_cast<std::size_t>(base)];
}

sf::Vector2f BaseballField::getPitcherPosition() const {
    return pitcherPosition_;
}

sf::Vector2f BaseballField::getBatterPosition() const {
    return batterPosition_;
}

sf::Vector2f BaseballField::getCatcherPosition() const {
    return catcherPosition_;
}

void BaseballField::buildGeometry() {
    const sf::Vector2u size = window_.getSize();
    const float width = static_cast<float>(size.x);
    const float height = static_cast<float>(size.y);

    // Dark green full-screen background.
    background_.setSize({ width, height });
    background_.setPosition({ 0.f, 0.f });
    background_.setFillColor(sf::Color(20, 85, 20));

    // Outfield arc (lighter green) behind the diamond.
    outfieldArc_.setFillColor(sf::Color(35, 120, 35));
    outfieldArc_.setOrigin({ outfieldArc_.getRadius(), outfieldArc_.getRadius() });
    outfieldArc_.setScale({ 1.8f, 1.0f });
    outfieldArc_.setPosition({ width * 0.5f, height * 0.48f });

    // Diamond placement in lower half.
    const sf::Vector2f home(width * 0.5f, height * 0.76f);
    const sf::Vector2f first(width * 0.64f, height * 0.60f);
    const sf::Vector2f second(width * 0.5f, height * 0.44f);
    const sf::Vector2f third(width * 0.36f, height * 0.60f);

    basePositions_[0] = home;
    basePositions_[1] = first;
    basePositions_[2] = second;
    basePositions_[3] = third;

    diamond_.setPointCount(4);
    diamond_.setPoint(0, home);
    diamond_.setPoint(1, first);
    diamond_.setPoint(2, second);
    diamond_.setPoint(3, third);
    diamond_.setFillColor(sf::Color(150, 115, 75));
    diamond_.setOutlineThickness(2.f);
    diamond_.setOutlineColor(sf::Color::White);

    // Base squares (20x20) centered on each base point.
    for (std::size_t i = 0; i < baseSquares_.size(); ++i) {
        baseSquares_[i].setSize({ 20.f, 20.f });
        baseSquares_[i].setOrigin({ 10.f, 10.f });
        baseSquares_[i].setPosition(basePositions_[i]);
        baseSquares_[i].setFillColor(sf::Color::White);
    }

    // Home plate as a pentagon near home position.
    homePlate_.setPointCount(5);
    homePlate_.setPoint(0, { 0.f, 0.f });
    homePlate_.setPoint(1, { 20.f, 0.f });
    homePlate_.setPoint(2, { 20.f, 12.f });
    homePlate_.setPoint(3, { 10.f, 20.f });
    homePlate_.setPoint(4, { 0.f, 12.f });
    homePlate_.setFillColor(sf::Color::White);
    homePlate_.setPosition({ home.x - 10.f, home.y - 10.f });

    // Pitcher's mound in center of diamond.
    pitcherPosition_ = { (home.x + second.x) * 0.5f, (home.y + second.y) * 0.5f };
    pitcherMound_.setRadius(12.f);
    pitcherMound_.setOrigin({ 12.f, 12.f });
    pitcherMound_.setPosition(pitcherPosition_);
    pitcherMound_.setFillColor(sf::Color(120, 80, 45));

    // Batter/Catcher helper positions.
    batterPosition_ = { home.x + 26.f, home.y + 8.f };
    catcherPosition_ = { home.x, home.y - 28.f };

    // Foul lines from home to top corners.
    foulLines_[0].position = home;
    foulLines_[1].position = { 0.f, 0.f };
    foulLines_[2].position = home;
    foulLines_[3].position = { width, 0.f };

    foulLines_[0].color = sf::Color::White;
    foulLines_[1].color = sf::Color::White;
    foulLines_[2].color = sf::Color::White;
    foulLines_[3].color = sf::Color::White;
}

} // namespace tungtung
