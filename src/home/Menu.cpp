#include "Menu.h"
#include "Settings.h"

#include <algorithm>

namespace corezone {

    // ─────────────────────────────────────────────────────────────────────────
    // MenuItem
    // ─────────────────────────────────────────────────────────────────────────

    MenuItem::MenuItem(const std::string& label, float width, float height)
        : label_(label), width_(width), height_(height) {
        background_.setSize({ width_, height_ });
        background_.setFillColor(sf::Color(15, 15, 15));
        background_.setOutlineColor(sf::Color::White);
        background_.setOutlineThickness(2.f);
    }

    void MenuItem::setPosition(sf::Vector2f pos) {
        position_ = pos;
        background_.setPosition(position_);
    }

    void MenuItem::setHovered(bool hovered) {
        hovered_ = hovered;
    }

    void MenuItem::applyGlow() {
        const auto alpha = static_cast<std::uint8_t>(std::clamp(glowAlpha_, 0.f, 255.f));
        if (hovered_) {
            background_.setFillColor(sf::Color::White);
            background_.setOutlineColor(sf::Color::White);
        }
        else {
            background_.setFillColor(sf::Color(15, 15, 15));
            background_.setOutlineColor(sf::Color(255, 255, 255, alpha > 80 ? alpha : 80));
        }
    }

    void MenuItem::update(float deltaTime) {
        const float target = hovered_ ? 255.f : 80.f;
        const float speed = 480.f;

        if (glowAlpha_ < target) {
            glowAlpha_ = std::min(target, glowAlpha_ + speed * deltaTime);
        }
        else if (glowAlpha_ > target) {
            glowAlpha_ = std::max(target, glowAlpha_ - speed * deltaTime);
        }

        applyGlow();
    }

    void MenuItem::draw(sf::RenderWindow& window, const sf::Font& font) {
        window.draw(background_);

        sf::Text text(font, label_, 22);
        text.setStyle(sf::Text::Bold);

        if (hovered_) {
            text.setFillColor(sf::Color::Black);
        }
        else {
            text.setFillColor(sf::Color::White);
        }

        const auto bounds = text.getLocalBounds();
        text.setPosition({
            position_.x + (width_ - bounds.size.x) * 0.5f - bounds.position.x,
            position_.y + (height_ - bounds.size.y) * 0.5f - bounds.position.y - 2.f
            });

        window.draw(text);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Menu
    // ─────────────────────────────────────────────────────────────────────────

    Menu::Menu(sf::RenderWindow& window, const sf::Font& font)
        : font_(font), window_(window), titleText_(font_, "MENU", 30) {
        mainMenuItems_.emplace_back("SETTINGS");
        mainMenuItems_.emplace_back("QUIT");
        settingsMenuItems_.emplace_back("BACK");

        overlay_.setFillColor(sf::Color(0, 0, 0, 150));

        panel_.setFillColor(sf::Color(10, 10, 10));
        panel_.setOutlineColor(sf::Color::White);
        panel_.setOutlineThickness(2.f);

        titleText_.setFillColor(sf::Color::White);
        titleText_.setStyle(sf::Text::Bold);

        repositionItems();
    }

    void Menu::repositionItems() {
        const auto ws = window_.getSize();
        const float ww = static_cast<float>(ws.x);
        const float wh = static_cast<float>(ws.y);

        overlay_.setSize({ ww, wh });

        panel_.setSize({ 420.f, 320.f });
        panel_.setPosition({ (ww - panel_.getSize().x) * 0.5f, (wh - panel_.getSize().y) * 0.5f });

        const auto titleBounds = titleText_.getLocalBounds();
        titleText_.setPosition({
            panel_.getPosition().x + (panel_.getSize().x - titleBounds.size.x) * 0.5f - titleBounds.position.x,
            panel_.getPosition().y + 22.f
            });

        auto placeItems = [this](std::vector<MenuItem>& items) {
            const float itemHeight = 50.f;
            const float spacing = 14.f;
            const float total = static_cast<float>(items.size()) * itemHeight +
                static_cast<float>(items.size() - 1) * spacing;

            float y = panel_.getPosition().y + (panel_.getSize().y - total) * 0.5f + 20.f;
            const float x = panel_.getPosition().x + (panel_.getSize().x - 250.f) * 0.5f;

            for (auto& item : items) {
                item.setPosition({ x, y });
                y += itemHeight + spacing;
            }
            };

        placeItems(mainMenuItems_);
        placeItems(settingsMenuItems_);
    }

    void Menu::activateMainMenu() {
        state_ = MenuState::MainMenu;
        titleText_.setString("MENU");
        hoveredIndex_ = 0;
    }

    void Menu::activateSettings() {
        state_ = MenuState::Settings;
        titleText_.setString("SETTINGS");
        settingsHoveredIndex_ = 0;
    }

    void Menu::toggle() {
        if (state_ == MenuState::Closed) open();
        else close();
    }

    void Menu::open() {
        activateMainMenu();
        repositionItems();
    }

    void Menu::close() {
        state_ = MenuState::Closed;
    }

    void Menu::selectMainItem(int index) {
        if (index == 0) {
            if (onSettings) onSettings();
            activateSettings();
        }
        else if (index == 1) {
            if (onQuit) onQuit();
        }
    }

    void Menu::handleInput(const sf::Event& event) {
        if (state_ == MenuState::Closed) return;

        if (event.is<sf::Event::KeyPressed>()) {
            const auto* key = event.getIf<sf::Event::KeyPressed>();

            if (key->code == sf::Keyboard::Key::Escape) {
                if (state_ == MenuState::Settings) {
                    activateMainMenu();
                }
                else {
                    close();
                }
                return;
            }

            if (state_ == MenuState::MainMenu) {
                if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) {
                    hoveredIndex_ = (hoveredIndex_ + 1) % static_cast<int>(mainMenuItems_.size());
                }
                else if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W) {
                    hoveredIndex_ = (hoveredIndex_ - 1 + static_cast<int>(mainMenuItems_.size())) % static_cast<int>(mainMenuItems_.size());
                }
                else if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) {
                    selectMainItem(hoveredIndex_);
                }
            }
            else if (state_ == MenuState::Settings) {
                if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) {
                    activateMainMenu();
                }
            }
        }
    }

    void Menu::handleMouseMove(sf::Vector2f mousePos) {
        if (state_ == MenuState::Closed) return;

        if (state_ == MenuState::MainMenu) {
            for (int i = 0; i < static_cast<int>(mainMenuItems_.size()); ++i) {
                const bool hover = mainMenuItems_[i].getBackground().getGlobalBounds().contains(mousePos);
                if (hover) hoveredIndex_ = i;
            }
        }
        else if (state_ == MenuState::Settings) {
            for (int i = 0; i < static_cast<int>(settingsMenuItems_.size()); ++i) {
                const bool hover = settingsMenuItems_[i].getBackground().getGlobalBounds().contains(mousePos);
                if (hover) settingsHoveredIndex_ = i;
            }
        }
    }

    void Menu::handleMouseClick(sf::Vector2f mousePos) {
        if (state_ == MenuState::Closed) return;

        if (state_ == MenuState::MainMenu) {
            for (int i = 0; i < static_cast<int>(mainMenuItems_.size()); ++i) {
                if (mainMenuItems_[i].getBackground().getGlobalBounds().contains(mousePos)) {
                    selectMainItem(i);
                    return;
                }
            }
        }
        else if (state_ == MenuState::Settings) {
            for (int i = 0; i < static_cast<int>(settingsMenuItems_.size()); ++i) {
                if (settingsMenuItems_[i].getBackground().getGlobalBounds().contains(mousePos)) {
                    activateMainMenu();
                    return;
                }
            }
        }
    }

    void Menu::update(float deltaTime) {
        if (state_ == MenuState::Closed) return;

        repositionItems();

        if (state_ == MenuState::MainMenu) {
            for (int i = 0; i < static_cast<int>(mainMenuItems_.size()); ++i) {
                mainMenuItems_[i].setHovered(i == hoveredIndex_);
                mainMenuItems_[i].update(deltaTime);
            }
        }
        else if (state_ == MenuState::Settings) {
            for (int i = 0; i < static_cast<int>(settingsMenuItems_.size()); ++i) {
                settingsMenuItems_[i].setHovered(i == settingsHoveredIndex_);
                settingsMenuItems_[i].update(deltaTime);
            }
        }
    }

    void Menu::draw() {
        if (state_ == MenuState::Closed) return;

        window_.draw(overlay_);
        window_.draw(panel_);
        window_.draw(titleText_);

        if (state_ == MenuState::MainMenu) {
            for (auto& item : mainMenuItems_) {
                item.draw(window_, font_);
            }
        }
        else if (state_ == MenuState::Settings) {
            for (auto& item : settingsMenuItems_) {
                item.draw(window_, font_);
            }
        }
    }

    float Menu::getMasterVolume() const {
        return settings_ ? settings_->getMasterVolume() : 0.f;
    }

    float Menu::getEffectVolume() const {
        return settings_ ? settings_->getEffectVolume() : 0.f;
    }

} // namespace corezone
