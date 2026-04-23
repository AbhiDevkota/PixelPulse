#ifndef SETTINGS_H
#define SETTINGS_H

#include <SFML/Graphics.hpp>
#include <string>

namespace corezone {

    class VolumeBar {
    private:
        sf::RectangleShape background_;
        sf::RectangleShape fill_;
        sf::RectangleShape knob_;
        std::string label_;
        sf::Text labelText_;

        float volume_ = 70.f;  // 0-100
        float minVolume_ = 0.f;
        float maxVolume_ = 100.f;
        bool isDragging_ = false;

        const float BAR_WIDTH = 300.f;
        const float BAR_HEIGHT = 20.f;

    public:
        VolumeBar(const sf::Font& font, const std::string& label);

        void setPosition(sf::Vector2f pos);
        void setVolume(float vol);
        float getVolume() const { return volume_; }

        void handleMousePress(sf::Vector2f mousePos);
        void handleMouseRelease();
        void handleMouseMove(sf::Vector2f mousePos);

        bool isHovered(sf::Vector2f mousePos) const;
        bool isDragging() const { return isDragging_; }

        void update();
        void draw(sf::RenderWindow& window);
    };

    class Settings {
    private:
        VolumeBar masterVolumeBar_;
        VolumeBar effectVolumeBar_;

        sf::Font& font_;
        sf::RenderWindow& window_;

        // UI State
        bool showSettings_ = false;
        int focusedSlider_ = 0;  // 0 = master, 1 = effect

    public:
        Settings(sf::RenderWindow& window, const sf::Font& font);

        void show() { showSettings_ = true; }
        void hide() { showSettings_ = false; }
        bool isVisible() const { return showSettings_; }

        float getMasterVolume() const { return masterVolumeBar_.getVolume(); }
        float getEffectVolume() const { return effectVolumeBar_.getVolume(); }

        void setMasterVolume(float vol) { masterVolumeBar_.setVolume(vol); }
        void setEffectVolume(float vol) { effectVolumeBar_.setVolume(vol); }

        void handleInput(const sf::Event& event);
        void update();
        void draw();
    };

} // namespace corezone

#endif
