#include "Settings.h"

#include <algorithm>

namespace corezone {

    Settings::Settings(sf::RenderWindow& window, sf::Font& font)
        : window_(window),
          font_(font),
          titleText_(font_, "SETTINGS", 28),
          masterLabel_(font_, "MASTER VOLUME", 18),
          effectLabel_(font_, "EFFECT VOLUME", 18)
    {
        overlay_.setFillColor(sf::Color(0, 0, 0, 170));

        panel_.setFillColor(sf::Color(15, 15, 15));
        panel_.setOutlineColor(sf::Color::White);
        panel_.setOutlineThickness(2.f);

        masterTrack_.setFillColor(sf::Color(40, 40, 40));
        masterTrack_.setOutlineColor(sf::Color::White);
        masterTrack_.setOutlineThickness(1.f);

        effectTrack_.setFillColor(sf::Color(40, 40, 40));
        effectTrack_.setOutlineColor(sf::Color::White);
        effectTrack_.setOutlineThickness(1.f);

        masterKnob_.setSize({ 14.f, 26.f });
        masterKnob_.setFillColor(sf::Color::White);
        masterKnob_.setOrigin({ 7.f, 13.f });

        effectKnob_.setSize({ 14.f, 26.f });
        effectKnob_.setFillColor(sf::Color::White);
        effectKnob_.setOrigin({ 7.f, 13.f });

        titleText_.setFillColor(sf::Color::White);
        masterLabel_.setFillColor(sf::Color(220, 220, 220));
        effectLabel_.setFillColor(sf::Color(220, 220, 220));

        updateLayout();
        syncKnobsFromValues();
    }

    void Settings::initialize() {
        if (onMasterVolumeChange) onMasterVolumeChange(masterVolume_);
        if (onEffectVolumeChange) onEffectVolumeChange(effectVolume_);
    }

    void Settings::show() {
        visible_ = true;
        updateLayout();
        syncKnobsFromValues();
    }

    void Settings::hide() {
        visible_ = false;
        resetState();
    }

    bool Settings::isVisible() const {
        return visible_;
    }

    void Settings::update() {
        if (!visible_) return;

        updateLayout();

        if (draggingMaster_ || draggingEffect_) {
            const sf::Vector2i mousePix = sf::Mouse::getPosition(window_);
            const sf::Vector2f mousePos = window_.mapPixelToCoords(mousePix);
            if (draggingMaster_) updateValueFromMouse(mousePos, true);
            if (draggingEffect_) updateValueFromMouse(mousePos, false);
        }
    }

    void Settings::draw() {
        if (!visible_) return;

        window_.draw(overlay_);
        window_.draw(panel_);
        window_.draw(titleText_);

        window_.draw(masterLabel_);
        window_.draw(masterTrack_);
        window_.draw(masterKnob_);

        window_.draw(effectLabel_);
        window_.draw(effectTrack_);
        window_.draw(effectKnob_);
    }

    void Settings::handleMousePress(sf::Vector2f mousePos) {
        if (!visible_) return;

        if (masterKnob_.getGlobalBounds().contains(mousePos) || masterTrack_.getGlobalBounds().contains(mousePos)) {
            draggingMaster_ = true;
            updateValueFromMouse(mousePos, true);
        }

        if (effectKnob_.getGlobalBounds().contains(mousePos) || effectTrack_.getGlobalBounds().contains(mousePos)) {
            draggingEffect_ = true;
            updateValueFromMouse(mousePos, false);
        }
    }

    void Settings::handleMouseRelease() {
        draggingMaster_ = false;
        draggingEffect_ = false;
    }

    void Settings::resetState() {
        draggingMaster_ = false;
        draggingEffect_ = false;
    }

    float Settings::getMasterVolume() const {
        return masterVolume_;
    }

    float Settings::getEffectVolume() const {
        return effectVolume_;
    }

    void Settings::updateLayout() {
        const auto size = window_.getSize();
        const float w = static_cast<float>(size.x);
        const float h = static_cast<float>(size.y);

        overlay_.setSize({ w, h });

        panel_.setSize({ 560.f, 320.f });
        panel_.setPosition({ (w - 560.f) * 0.5f, (h - 320.f) * 0.5f });

        const float px = panel_.getPosition().x;
        const float py = panel_.getPosition().y;

        titleText_.setPosition({ px + 32.f, py + 24.f });

        masterLabel_.setPosition({ px + 32.f, py + 96.f });
        masterTrack_.setPosition({ px + 32.f, py + 132.f });
        masterTrack_.setSize({ 496.f, 10.f });

        effectLabel_.setPosition({ px + 32.f, py + 186.f });
        effectTrack_.setPosition({ px + 32.f, py + 222.f });
        effectTrack_.setSize({ 496.f, 10.f });

        syncKnobsFromValues();
    }

    void Settings::syncKnobsFromValues() {
        const auto placeKnob = [](sf::RectangleShape& knob, const sf::RectangleShape& track, float value) {
            const sf::FloatRect b = track.getGlobalBounds();
            const float t = std::clamp(value / 100.f, 0.f, 1.f);
            knob.setPosition({ b.position.x + (b.size.x * t), b.position.y + b.size.y * 0.5f });
        };

        placeKnob(masterKnob_, masterTrack_, masterVolume_);
        placeKnob(effectKnob_, effectTrack_, effectVolume_);
    }

    void Settings::updateValueFromMouse(sf::Vector2f mousePos, bool masterSlider) {
        sf::RectangleShape& track = masterSlider ? masterTrack_ : effectTrack_;
        const sf::FloatRect b = track.getGlobalBounds();

        const float t = std::clamp((mousePos.x - b.position.x) / b.size.x, 0.f, 1.f);
        const float value = t * 100.f;

        if (masterSlider) {
            masterVolume_ = value;
            if (onMasterVolumeChange) onMasterVolumeChange(masterVolume_);
        }
        else {
            effectVolume_ = value;
            if (onEffectVolumeChange) onEffectVolumeChange(effectVolume_);
        }

        syncKnobsFromValues();
    }

} // namespace corezone
