#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

namespace corezone {

    // Forward declaration so Menu can hold a non-owning pointer to Settings
    class Settings;

    // ─────────────────────────────────────────────────────────────────────────
    // MenuItem
    // ─────────────────────────────────────────────────────────────────────────
    class MenuItem {
    private:
        std::string          label_;
        sf::RectangleShape   background_;
        sf::Vector2f         position_;
        float                width_;
        float                height_;
        bool                 hovered_ = false;

        // Smooth hover animation
        float                glowAlpha_ = 0.f;   // 0–255, drives outline brightness

        void applyGlow();

    public:
        MenuItem(const std::string& label, float width = 250.f, float height = 50.f);

        void setPosition(sf::Vector2f pos);
        void setHovered(bool hovered);
        bool isHovered() const { return hovered_; }

        // deltaTime-based animation tick
        void update(float deltaTime);

        // Legacy no-arg overload kept so existing callers still compile
        void update() { update(0.f); }

        void draw(sf::RenderWindow& window, const sf::Font& font);

        const sf::RectangleShape& getBackground() const { return background_; }
        const std::string& getLabel()       const { return label_; }
    };

    // ─────────────────────────────────────────────────────────────────────────
    // Menu
    // ─────────────────────────────────────────────────────────────────────────
    class Menu {
    public:
        enum class MenuState { Closed, MainMenu, Settings };

    private:
        std::vector<MenuItem>  mainMenuItems_;
        std::vector<MenuItem>  settingsMenuItems_;   // currently: "BACK"
        MenuState              state_ = MenuState::Closed;

        int hoveredIndex_ = 0;
        int settingsHoveredIndex_ = 0;

        sf::Font& font_;
        sf::RenderWindow& window_;

        // Non-owning pointer to the shared Settings instance (injected via
        // setSettings()).  May be nullptr if not wired up.
        Settings* settings_ = nullptr;

        // Semi-transparent full-screen overlay
        sf::RectangleShape overlay_;

        // Panel background drawn behind menu items
        sf::RectangleShape panel_;

        // Title text drawn at the top of the panel
        sf::Text titleText_;

        // ── helpers ──────────────────────────────────────────────────────────
        void repositionItems();
        void activateMainMenu();
        void activateSettings();
        void selectMainItem(int index);

        // ── Controller support ─────────────────────────────────────────────────
        sf::Clock joystickDelayClock_;
        void handleControllerNavigation();
        // ─────────────────────────────────────────────────────────────────────────

    public:
        Menu(sf::RenderWindow& window, const sf::Font& font);

        // Wire up the Settings instance owned by HomeScreen
        void setSettings(Settings* settings) { settings_ = settings; }

        MenuState getState() const { return state_; }
        void toggle();
        void open();
        void close();

        void handleInput(const sf::Event& event);
        void handleMouseMove(sf::Vector2f mousePos);
        void handleMouseClick(sf::Vector2f mousePos);   // ← NEW: call on left-button release

        void update(float deltaTime);
        void update() { update(0.f); }  // backward-compat overload

        void draw();

        // Callbacks for menu actions
        std::function<void()>      onSettings;
        std::function<void()>      onQuit;
        std::function<void(float)> onMasterVolumeChange;
        std::function<void(float)> onEffectVolumeChange;

        // Returns live values from the wired Settings, or 0 if not wired
        float getMasterVolume() const;
        float getEffectVolume() const;
    };

} // namespace corezone

#endif