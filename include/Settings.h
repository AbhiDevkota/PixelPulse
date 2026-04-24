#ifndef SETTINGS_H
#define SETTINGS_H

#include <SFML/Graphics.hpp>
#include <functional>

namespace corezone {

class Settings {
private:
    sf::RenderWindow& window_;
    sf::Font& font_;

    bool visible_ = false;
    bool draggingMaster_ = false;
    bool draggingEffect_ = false;

    float masterVolume_ = 70.f;
    float effectVolume_ = 100.f;

    sf::RectangleShape overlay_;
    sf::RectangleShape panel_;

    sf::RectangleShape masterTrack_;
    sf::RectangleShape effectTrack_;
    sf::RectangleShape masterKnob_;
    sf::RectangleShape effectKnob_;

    sf::Text titleText_;
    sf::Text masterLabel_;
    sf::Text effectLabel_;

    void updateLayout();
    void syncKnobsFromValues();
    void updateValueFromMouse(sf::Vector2f mousePos, bool masterSlider);

public:
    Settings(sf::RenderWindow& window, sf::Font& font);

    void initialize();
    void show();
    void hide();
    bool isVisible() const;

    void update();
    void draw();

    void handleMousePress(sf::Vector2f mousePos);
    void handleMouseRelease();
    void resetState();

    float getMasterVolume() const;
    float getEffectVolume() const;

    std::function<void(float)> onMasterVolumeChange;
    std::function<void(float)> onEffectVolumeChange;
};

} // namespace corezone

#endif