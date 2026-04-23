#include "Menu.h"
#include <iostream>

namespace corezone {

    MenuItem::MenuItem(const std::string& label, float width, float height)
        : label_(label), width_(width), height_(height) {

        background_.setSize({ width, height });
        background_.setFillColor(sf::Color(50, 50, 50));
        background_.setOutlineColor(sf::Color(100, 100, 100));
        background_.setOutlineThickness(2.f);
    }

    void MenuItem::setPosition(sf::Vector2f pos) {
        position_ = pos;
        background_.setPosition(pos);
    }

    void MenuItem::setHovered(bool hovered) {
        hovered_ = hovered;
        if (hovered) {
            background_.setFillColor(sf::Color(80, 80, 80));
            background_.setOutlineColor(sf::Color(255, 255, 255));
            background_.setOutlineThickness(3.f);
        } else {
            background_.setFillColor(sf::Color(50, 50, 50));
            background_.setOutlineColor(sf::Color(100, 100, 100));
            background_.setOutlineThickness(2.f);
        }
    }

    void MenuItem::update() {
        // Animation could be added here
    }

    void MenuItem::draw(sf::RenderWindow& window, const sf::Font& font) {
        window.draw(background_);

        sf::Text text(font, label_);
        text.setCharacterSize(24);
        text.setFillColor(hovered_ ? sf::Color(255, 255, 255) : sf::Color(200, 200, 200));

        auto bounds = text.getLocalBounds();
        text.setPosition({
            position_.x + (width_ - bounds.size.x) / 2.f - bounds.position.x,
            position_.y + (height_ - bounds.size.y) / 2.f - bounds.position.y
        });

        window.draw(text);
    }

    // ─────────────────────────────────────────────────────────────────

    Menu::Menu(sf::RenderWindow& window, const sf::Font& font)
        : font_(const_cast<sf::Font&>(font)), window_(window) {

        // Create main menu items
        mainMenuItems_.emplace_back("SETTINGS", 250.f, 50.f);
        mainMenuItems_.emplace_back("QUIT", 250.f, 50.f);

        // Create settings menu items (volume controls will be drawn separately)
        settingsMenuItems_.emplace_back("BACK", 250.f, 50.f);

        // Setup overlay
        auto size = window_.getSize();
        overlay_.setSize({ static_cast<float>(size.x), static_cast<float>(size.y) });
        overlay_.setFillColor(sf::Color(0, 0, 0, 180));

        // Setup menu item positions
        float centerX = (static_cast<float>(size.x) - 250.f) / 2.f;
        float centerY = (static_cast<float>(size.y) - 120.f) / 2.f;

        mainMenuItems_[0].setPosition({ centerX, centerY });
        mainMenuItems_[1].setPosition({ centerX, centerY + 70.f });
    }

    void Menu::toggle() {
        if (state_ == MenuState::Closed) {
            open();
        } else {
            close();
        }
    }

    void Menu::open() {
        state_ = MenuState::MainMenu;
        hoveredIndex_ = 0;
        mainMenuItems_[0].setHovered(true);
    }

    void Menu::close() {
        state_ = MenuState::Closed;
        for (auto& item : mainMenuItems_) {
            item.setHovered(false);
        }
        for (auto& item : settingsMenuItems_) {
            item.setHovered(false);
        }
    }

    void Menu::handleInput(const sf::Event& event) {
        if (state_ == MenuState::Closed) return;

        if (event.is<sf::Event::KeyPressed>()) {
            const auto* key = event.getIf<sf::Event::KeyPressed>();

            if (state_ == MenuState::MainMenu) {
                switch (key->code) {
                case sf::Keyboard::Key::Up:
                case sf::Keyboard::Key::W:
                    mainMenuItems_[hoveredIndex_].setHovered(false);
                    hoveredIndex_ = (hoveredIndex_ - 1 + static_cast<int>(mainMenuItems_.size())) % static_cast<int>(mainMenuItems_.size());
                    mainMenuItems_[hoveredIndex_].setHovered(true);
                    break;

                case sf::Keyboard::Key::Down:
                case sf::Keyboard::Key::S:
                    mainMenuItems_[hoveredIndex_].setHovered(false);
                    hoveredIndex_ = (hoveredIndex_ + 1) % static_cast<int>(mainMenuItems_.size());
                    mainMenuItems_[hoveredIndex_].setHovered(true);
                    break;

                case sf::Keyboard::Key::Enter:
                case sf::Keyboard::Key::Space:
                    if (hoveredIndex_ == 0) {  // Settings
                        if (onSettings) onSettings();
                        state_ = MenuState::Settings;
                    } else if (hoveredIndex_ == 1) {  // Quit
                        if (onQuit) onQuit();
                    }
                    break;

                case sf::Keyboard::Key::Escape:
                    close();
                    break;

                default: break;
                }
            }
            else if (state_ == MenuState::Settings) {
                if (key->code == sf::Keyboard::Key::Escape) {
                    state_ = MenuState::MainMenu;
                    hoveredIndex_ = 0;
                    mainMenuItems_[0].setHovered(true);
                }
            }
        }
    }

    void Menu::handleMouseMove(sf::Vector2f mousePos) {
        if (state_ == MenuState::Closed) return;

        if (state_ == MenuState::MainMenu) {
            for (size_t i = 0; i < mainMenuItems_.size(); ++i) {
                const auto& bg = mainMenuItems_[i].getBackground();
                if (bg.getGlobalBounds().contains(mousePos)) {
                    if (hoveredIndex_ != static_cast<int>(i)) {
                        mainMenuItems_[hoveredIndex_].setHovered(false);
                        hoveredIndex_ = static_cast<int>(i);
                        mainMenuItems_[hoveredIndex_].setHovered(true);
                    }
                    return;
                }
            }
        }
    }

    void Menu::update() {
        for (auto& item : mainMenuItems_) {
            item.update();
        }
        for (auto& item : settingsMenuItems_) {
            item.update();
        }
    }

    void Menu::draw() {
        if (state_ == MenuState::Closed) return;

        window_.draw(overlay_);

        if (state_ == MenuState::MainMenu) {
            for (auto& item : mainMenuItems_) {
                item.draw(window_, font_);
            }
        }
    }

    float Menu::getMasterVolume() const {
        return 0.f;  // To be implemented with Settings
    }

    float Menu::getEffectVolume() const {
        return 0.f;  // To be implemented with Settings
    }

} // namespace corezone
