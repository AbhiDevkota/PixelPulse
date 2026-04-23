#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

namespace corezone {

    class MenuItem {
    private:
        std::string label_;
        sf::RectangleShape background_;
        sf::Vector2f position_;
        float width_;
        float height_;
        bool hovered_ = false;

    public:
        MenuItem(const std::string& label, float width = 250.f, float height = 50.f);

        void setPosition(sf::Vector2f pos);
        void setHovered(bool hovered);
        bool isHovered() const { return hovered_; }

        void update();
        void draw(sf::RenderWindow& window, const sf::Font& font);

        const sf::RectangleShape& getBackground() const { return background_; }
        const std::string& getLabel() const { return label_; }
    };

    class Menu {
    public:
        enum class MenuState { Closed, MainMenu, Settings };

    private:
        std::vector<MenuItem> mainMenuItems_;
        std::vector<MenuItem> settingsMenuItems_;
        MenuState state_ = MenuState::Closed;

        int hoveredIndex_ = 0;
        int settingsHoveredIndex_ = 0;

        sf::Font& font_;
        sf::RenderWindow& window_;

        // Semi-transparent overlay
        sf::RectangleShape overlay_;

    public:
        Menu(sf::RenderWindow& window, const sf::Font& font);

        MenuState getState() const { return state_; }
        void toggle();
        void open();
        void close();

        void handleInput(const sf::Event& event);
        void handleMouseMove(sf::Vector2f mousePos);

        void update();
        void draw();

        // Callbacks for menu actions
        std::function<void()> onSettings;
        std::function<void()> onQuit;
        std::function<void(float)> onMasterVolumeChange;
        std::function<void(float)> onEffectVolumeChange;

        float getMasterVolume() const;
        float getEffectVolume() const;
    };

} // namespace corezone

#endif
