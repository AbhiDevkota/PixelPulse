#include "tungtungleague/common/AnimatedLine.h"

namespace tungtung {

AnimatedLine::AnimatedLine(sf::RenderWindow& window)
    : window_(window), band_(sf::PrimitiveType::TriangleStrip, 6), posY_(-lineHeight_) {
    rebuildBand();
}

void AnimatedLine::update(float dt) {
    // Scroll band downward at a constant speed.
    posY_ += speed_ * dt;

    // Loop from the top once it moves below the visible window.
    const float windowHeight = static_cast<float>(window_.getSize().y);
    if (posY_ > windowHeight) {
        posY_ = -lineHeight_;
    }

    rebuildBand();
}

void AnimatedLine::draw(sf::RenderWindow& window) {
    window.draw(band_);
}

void AnimatedLine::rebuildBand() {
    const float width = static_cast<float>(window_.getSize().x);

    const float topY = posY_;
    const float midY = posY_ + (lineHeight_ * 0.5f);
    const float bottomY = posY_ + lineHeight_;

    // Triangle strip: top edge (alpha 0), center line (alpha 255), bottom edge (alpha 0).
    band_[0].position = { 0.f, topY };
    band_[1].position = { width, topY };
    band_[2].position = { 0.f, midY };
    band_[3].position = { width, midY };
    band_[4].position = { 0.f, bottomY };
    band_[5].position = { width, bottomY };

    const sf::Color transparentWhite(255, 255, 255, 0);
    const sf::Color centerWhite(255, 255, 255, 255);

    band_[0].color = transparentWhite;
    band_[1].color = transparentWhite;
    band_[2].color = centerWhite;
    band_[3].color = centerWhite;
    band_[4].color = transparentWhite;
    band_[5].color = transparentWhite;
}

} // namespace tungtung
