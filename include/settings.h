#ifndef SETTINGS_H
#define SETTINGS_H

#include <SFML/Graphics.hpp>
#include <string>
#include <functional>
#include "files.h"

namespace corezone {

    class VolumeBar {
    private:
        sf::RectangleShape background_;
        sf::RectangleShape fill_;
        sf::RectangleShape knob_;
        std::string label_;
        sf::Text labelText_;
        sf::Text volumeText_;  // Display volume percentage
        const sf::Font& font_;

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
        void resetDragging() { isDragging_ = false; }  // Reset dragging state
        void forceStopDragging();  // Force stop dragging

        void update();
        void draw(sf::RenderWindow& window);

        // Callback for volume changes
        std::function<void(float)> onVolumeChange;
    };

    class Settings {
    private:
        VolumeBar masterVolumeBar_;
        VolumeBar effectVolumeBar_;

        sf::Font& font_;
        sf::RenderWindow& window_;
        FileManager* fileManager_ = nullptr;  // Add FileManager pointer

        // UI State
        bool showSettings_ = false;
        int focusedSlider_ = 0;  // 0 = master, 1 = effect

        // Overlay to darken background
        sf::RectangleShape overlay_;

        void saveSettings();
        void loadSettings();
        void saveVolumeSettings();  //save using FileManager
        void loadVolumeSettings();  //load using FileManager

    public:
        Settings(sf::RenderWindow& window, const sf::Font& font);

        // Wire up the FileManager instance
        void setFileManager(FileManager* fm) { fileManager_ = fm; }

        void initialize();  // Call after wiring up callbacks
        void show() { showSettings_ = true; }
        void hide() { showSettings_ = false; }
        bool isVisible() const { return showSettings_; }

        float getMasterVolume() const { return masterVolumeBar_.getVolume(); }
        float getEffectVolume() const { return effectVolumeBar_.getVolume(); }

        void setMasterVolume(float vol);
        void setEffectVolume(float vol);

        void handleInput(const sf::Event& event);
        void handleMouseMove(sf::Vector2f mousePos);
        void handleMousePress(sf::Vector2f mousePos);  // Direct mouse press handling
        void handleMouseRelease();  // Direct mouse release handling
        void update();
        void draw();
        void resetState();  // Reset volume bars state when closing menu
        void forceStopAllDragging();  // Force stop all dragging operations

        // Callbacks for volume changes
        std::function<void(float)> onMasterVolumeChange;
        std::function<void(float)> onEffectVolumeChange;
    };

}

#endif