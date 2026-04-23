#include "Settings.h"
#include <cmath>
#include <iostream>

namespace corezone {

    VolumeBar::VolumeBar(const sf::Font& font, const std::string& label)
        : label_(label), labelText_(font, label) {

        // Label text
        labelText_.setCharacterSize(18);
        labelText_.setFillColor(sf::Color(200, 200, 200));

        // Background (track)
        background_.setSize({ BAR_WIDTH, BAR_HEIGHT });
        background_.setFillColor(sf::Color(30, 30, 30));
        background_.setOutlineColor(sf::Color(100, 100, 100));
        background_.setOutlineThickness(1.5f);

        // Fill (progress)
        fill_.setSize({ BAR_WIDTH * (volume_ / maxVolume_), BAR_HEIGHT });
        fill_.setFillColor(sf::Color(100, 200, 100));

        // Knob (slider handle)
        knob_.setSize({ 15.f, BAR_HEIGHT + 10.f });
        knob_.setFillColor(sf::Color(150, 255, 150));
        knob_.setOutlineColor(sf::Color(255, 255, 255));
        knob_.setOutlineThickness(1.5f);
    }

    void VolumeBar::setPosition(sf::Vector2f pos) {
        background_.setPosition(pos);
        fill_.setPosition(pos);

        float knobX = pos.x + (volume_ / maxVolume_) * BAR_WIDTH - 7.5f;
        knob_.setPosition({ knobX, pos.y - 5.f });

        labelText_.setPosition({ pos.x - 150.f, pos.y + 2.f });
    }

    void VolumeBar::setVolume(float vol) {
        volume_ = std::max(minVolume_, std::min(maxVolume_, vol));

        // Update fill width
        fill_.setSize({ BAR_WIDTH * (volume_ / maxVolume_), BAR_HEIGHT });

        // Update knob position
        auto bgPos = background_.getPosition();
        float knobX = bgPos.x + (volume_ / maxVolume_) * BAR_WIDTH - 7.5f;
        knob_.setPosition({ knobX, bgPos.y - 5.f });
    }

    void VolumeBar::handleMousePress(sf::Vector2f mousePos) {
        if (isHovered(mousePos)) {
            isDragging_ = true;
            handleMouseMove(mousePos);
        }
    }

    void VolumeBar::handleMouseRelease() {
        isDragging_ = false;
    }

    void VolumeBar::handleMouseMove(sf::Vector2f mousePos) {
        if (isDragging_) {
            auto bgPos = background_.getPosition();
            float relativeX = mousePos.x - bgPos.x;
            float newVolume = (relativeX / BAR_WIDTH) * maxVolume_;
            setVolume(newVolume);
        }
    }

    bool VolumeBar::isHovered(sf::Vector2f mousePos) const {
        sf::FloatRect bounds = background_.getGlobalBounds();
        bounds.position.y -= 10.f;
        bounds.size.y += 20.f;
        bounds.position.x -= 50.f;
        bounds.size.x += 100.f;
        return bounds.contains(mousePos);
    }

    void VolumeBar::update() {
        // Could add animation or other updates here
    }

    void VolumeBar::draw(sf::RenderWindow& window) {
        window.draw(labelText_);
        window.draw(background_);
        window.draw(fill_);
        window.draw(knob_);
    }

    // ─────────────────────────────────────────────────────────────────

    Settings::Settings(sf::RenderWindow& window, const sf::Font& font)
        : masterVolumeBar_(font, "Master Volume"),
          effectVolumeBar_(font, "Effect Volume"),
          font_(const_cast<sf::Font&>(font)),
          window_(window) {

        auto size = window_.getSize();
        float centerX = static_cast<float>(size.x) / 2.f;
        float centerY = static_cast<float>(size.y) / 2.f;

        // Position volume bars
        masterVolumeBar_.setPosition({ centerX - 150.f, centerY - 80.f });
        effectVolumeBar_.setPosition({ centerX - 150.f, centerY + 20.f });
    }

    void Settings::handleInput(const sf::Event& event) {
        if (!showSettings_) return;

        if (event.is<sf::Event::MouseButtonPressed>()) {
            const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>();
            if (mouse->button == sf::Mouse::Button::Left) {
                auto mousePos = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
                masterVolumeBar_.handleMousePress(mousePos);
                effectVolumeBar_.handleMousePress(mousePos);
            }
        }
        else if (event.is<sf::Event::MouseButtonReleased>()) {
            const auto* mouse = event.getIf<sf::Event::MouseButtonReleased>();
            if (mouse->button == sf::Mouse::Button::Left) {
                masterVolumeBar_.handleMouseRelease();
                effectVolumeBar_.handleMouseRelease();
            }
        }
    }

    void Settings::update() {
        if (!showSettings_) return;

        auto mousePos = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
        masterVolumeBar_.handleMouseMove(mousePos);
        effectVolumeBar_.handleMouseMove(mousePos);

        masterVolumeBar_.update();
        effectVolumeBar_.update();
    }

    void Settings::draw() {
        if (!showSettings_) return;

        // Title
        sf::Text title(font_, "SETTINGS");
        title.setCharacterSize(36);
        title.setFillColor(sf::Color(255, 255, 255));

        auto size = window_.getSize();
        float centerX = static_cast<float>(size.x) / 2.f;
        auto bounds = title.getLocalBounds();
        title.setPosition({ centerX - bounds.size.x / 2.f, static_cast<float>(size.y) / 2.f - 180.f });

        window_.draw(title);

        // Draw volume bars
        masterVolumeBar_.draw(window_);
        effectVolumeBar_.draw(window_);

        // Draw back button
        sf::Text backText(font_, "Press ESC to go back");
        backText.setCharacterSize(14);
        backText.setFillColor(sf::Color(160, 160, 160));
        backText.setStyle(sf::Text::Italic);

        bounds = backText.getLocalBounds();
        backText.setPosition({ centerX - bounds.size.x / 2.f, static_cast<float>(size.y) - 80.f });
        window_.draw(backText);
    }

} // namespace corezone
