#include "Menu.h"
#include "Settings.h"   // needed to call settings_->show/hide/draw/etc.
#include <algorithm>    // std::clamp
#include <iostream>

namespace corezone {

    // =========================================================================
    // MenuItem
    // =========================================================================

    MenuItem::MenuItem(const std::string& label, float width, float height)
        : label_(label), width_(width), height_(height) {

        background_.setSize({ width, height });
        background_.setFillColor(sf::Color(40, 40, 40));
        background_.setOutlineColor(sf::Color(90, 90, 90));
        background_.setOutlineThickness(2.f);
    }

    void MenuItem::setPosition(sf::Vector2f pos) {
        position_ = pos;
        background_.setPosition(pos);
    }

    void MenuItem::setHovered(bool hovered) {
        hovered_ = hovered;
        // Snap glow immediately so there's no stale value when focus jumps
        glowAlpha_ = hovered ? 255.f : 0.f;
        applyGlow();
    }

    void MenuItem::applyGlow() {
        if (hovered_) {
            // Lerp fill from dark-grey toward a highlighted colour
            uint8_t g = static_cast<uint8_t>(40 + (glowAlpha_ / 255.f) * 55.f);
            background_.setFillColor(sf::Color(g, g, g + 20));
            background_.setOutlineColor(sf::Color(
                static_cast<uint8_t>(glowAlpha_),
                static_cast<uint8_t>(glowAlpha_),
                255));
            background_.setOutlineThickness(3.f);
        }
        else {
            background_.setFillColor(sf::Color(40, 40, 40));
            background_.setOutlineColor(sf::Color(90, 90, 90));
            background_.setOutlineThickness(2.f);
        }
    }

    void MenuItem::update(float deltaTime) {
        // Fade glow in/out at ~400 units/sec
        const float speed = 400.f;
        if (hovered_) {
            glowAlpha_ = std::min(255.f, glowAlpha_ + speed * deltaTime);
        }
        else {
            glowAlpha_ = std::max(0.f, glowAlpha_ - speed * deltaTime);
        }
        applyGlow();
    }

    void MenuItem::draw(sf::RenderWindow& window, const sf::Font& font) {
        window.draw(background_);

        sf::Text text(font, label_);
        text.setCharacterSize(24);
        text.setFillColor(hovered_ ? sf::Color(255, 255, 255) : sf::Color(190, 190, 200));

        auto bounds = text.getLocalBounds();
        text.setPosition({
            position_.x + (width_ - bounds.size.x) / 2.f - bounds.position.x,
            position_.y + (height_ - bounds.size.y) / 2.f - bounds.position.y
            });

        window.draw(text);
    }

    // =========================================================================
    // Menu  –  private helpers
    // =========================================================================

    void Menu::repositionItems() {
        auto size = window_.getSize();
        float cx = (static_cast<float>(size.x) - 250.f) / 2.f;

        // ── Main menu items (centred vertically as a block) ──────────────────
        const float itemH = 50.f;
        const float itemGap = 20.f;
        int  n = static_cast<int>(mainMenuItems_.size());
        float blockH = n * itemH + (n - 1) * itemGap;
        float startY = (static_cast<float>(size.y) - blockH) / 2.f;

        for (int i = 0; i < n; ++i) {
            mainMenuItems_[i].setPosition({ cx, startY + i * (itemH + itemGap) });
        }

        // ── Settings menu items (BACK button near the bottom) ────────────────
        float backY = static_cast<float>(size.y) - 110.f;
        for (auto& item : settingsMenuItems_) {
            item.setPosition({ cx, backY });
        }

        // ── Panel background ─────────────────────────────────────────────────
        const float panelW = 320.f;
        const float panelH = blockH + 120.f;
        panel_.setSize({ panelW, panelH });
        panel_.setPosition({
            (static_cast<float>(size.x) - panelW) / 2.f,
            startY - 60.f
            });
    }

    void Menu::activateMainMenu() {
        state_ = MenuState::MainMenu;
        hoveredIndex_ = 0;
        mainMenuItems_[0].setHovered(true);
        for (size_t i = 1; i < mainMenuItems_.size(); ++i)
            mainMenuItems_[i].setHovered(false);

        if (settings_) settings_->hide();
    }

    void Menu::activateSettings() {
        state_ = MenuState::Settings;
        settingsHoveredIndex_ = 0;
        if (!settingsMenuItems_.empty())
            settingsMenuItems_[0].setHovered(true);

        if (settings_) settings_->show();
        if (onSettings) onSettings();
    }

    // =========================================================================
    // Menu  –  public
    // =========================================================================

    Menu::Menu(sf::RenderWindow& window, const sf::Font& font)
        : font_(const_cast<sf::Font&>(font)),
        window_(window),
        titleText_(font, "PAUSED") {

        // ── Main menu items ───────────────────────────────────────────────────
        mainMenuItems_.emplace_back("RESUME", 250.f, 50.f);
        mainMenuItems_.emplace_back("SETTINGS", 250.f, 50.f);
        mainMenuItems_.emplace_back("QUIT", 250.f, 50.f);

        // ── Settings sub-menu items ───────────────────────────────────────────
        settingsMenuItems_.emplace_back("BACK", 250.f, 50.f);

        // ── Overlay ───────────────────────────────────────────────────────────
        auto size = window_.getSize();
        overlay_.setSize({ static_cast<float>(size.x), static_cast<float>(size.y) });
        overlay_.setFillColor(sf::Color(0, 0, 0, 160));

        // ── Panel ─────────────────────────────────────────────────────────────
        panel_.setFillColor(sf::Color(20, 20, 30, 220));
        panel_.setOutlineColor(sf::Color(80, 80, 120));
        panel_.setOutlineThickness(2.f);

        // ── Title ─────────────────────────────────────────────────────────────
        titleText_.setCharacterSize(32);
        titleText_.setFillColor(sf::Color(220, 220, 255));
        titleText_.setStyle(sf::Text::Bold);

        repositionItems();
    }

    // ── Open / close / toggle ────────────────────────────────────────────────

    void Menu::toggle() {
        if (state_ == MenuState::Closed) open();
        else                             close();
    }

    void Menu::open() {
        activateMainMenu();
    }

    void Menu::close() {
        state_ = MenuState::Closed;
        for (auto& item : mainMenuItems_)    item.setHovered(false);
        for (auto& item : settingsMenuItems_) item.setHovered(false);
        if (settings_) settings_->hide();
    }

    // ── Input ────────────────────────────────────────────────────────────────

    void Menu::handleInput(const sf::Event& event) {
        if (state_ == MenuState::Closed) return;

        // Forward mouse / drag events to Settings when in settings state
        if (state_ == MenuState::Settings && settings_) {
            settings_->handleInput(event);
        }

        if (!event.is<sf::Event::KeyPressed>()) return;
        const auto* key = event.getIf<sf::Event::KeyPressed>();

        // ── Main menu keyboard nav ────────────────────────────────────────────
        if (state_ == MenuState::MainMenu) {
            int n = static_cast<int>(mainMenuItems_.size());
            switch (key->code) {
            case sf::Keyboard::Key::Up:
            case sf::Keyboard::Key::W:
                mainMenuItems_[hoveredIndex_].setHovered(false);
                hoveredIndex_ = (hoveredIndex_ - 1 + n) % n;
                mainMenuItems_[hoveredIndex_].setHovered(true);
                break;

            case sf::Keyboard::Key::Down:
            case sf::Keyboard::Key::S:
                mainMenuItems_[hoveredIndex_].setHovered(false);
                hoveredIndex_ = (hoveredIndex_ + 1) % n;
                mainMenuItems_[hoveredIndex_].setHovered(true);
                break;

            case sf::Keyboard::Key::Enter:
            case sf::Keyboard::Key::Space:
                selectMainItem(hoveredIndex_);
                break;

            case sf::Keyboard::Key::Escape:
                close();
                break;

            default: break;
            }
        }
        // ── Settings keyboard nav ─────────────────────────────────────────────
        else if (state_ == MenuState::Settings) {
            switch (key->code) {
            case sf::Keyboard::Key::Escape:
            case sf::Keyboard::Key::Backspace:
                activateMainMenu();
                break;

            case sf::Keyboard::Key::Enter:
            case sf::Keyboard::Key::Space:
                // Only one settings item (BACK) — treat Enter as BACK
                if (settingsHoveredIndex_ == 0)
                    activateMainMenu();
                break;

                // Left / Right arrows adjust the focused slider
            case sf::Keyboard::Key::Left:
            case sf::Keyboard::Key::A:
                if (settings_) {
                    if (settingsHoveredIndex_ == 0) {
                        float v = std::max(0.f, settings_->getMasterVolume() - 5.f);
                        settings_->setMasterVolume(v);
                        if (onMasterVolumeChange) onMasterVolumeChange(v);
                    }
                    else {
                        float v = std::max(0.f, settings_->getEffectVolume() - 5.f);
                        settings_->setEffectVolume(v);
                        if (onEffectVolumeChange) onEffectVolumeChange(v);
                    }
                }
                break;

            case sf::Keyboard::Key::Right:
            case sf::Keyboard::Key::D:
                if (settings_) {
                    if (settingsHoveredIndex_ == 0) {
                        float v = std::min(100.f, settings_->getMasterVolume() + 5.f);
                        settings_->setMasterVolume(v);
                        if (onMasterVolumeChange) onMasterVolumeChange(v);
                    }
                    else {
                        float v = std::min(100.f, settings_->getEffectVolume() + 5.f);
                        settings_->setEffectVolume(v);
                        if (onEffectVolumeChange) onEffectVolumeChange(v);
                    }
                }
                break;

                // Tab / Up / Down cycle focused slider
            case sf::Keyboard::Key::Tab:
            case sf::Keyboard::Key::Up:
            case sf::Keyboard::Key::Down:
                settingsHoveredIndex_ = 1 - settingsHoveredIndex_; // toggle 0/1
                break;

            default: break;
            }
        }
    }

    void Menu::handleMouseMove(sf::Vector2f mousePos) {
        if (state_ == MenuState::Closed) return;

        if (state_ == MenuState::MainMenu) {
            for (size_t i = 0; i < mainMenuItems_.size(); ++i) {
                if (mainMenuItems_[i].getBackground().getGlobalBounds().contains(mousePos)) {
                    if (hoveredIndex_ != static_cast<int>(i)) {
                        mainMenuItems_[hoveredIndex_].setHovered(false);
                        hoveredIndex_ = static_cast<int>(i);
                        mainMenuItems_[hoveredIndex_].setHovered(true);
                    }
                    return;
                }
            }
        }
        else if (state_ == MenuState::Settings) {
            // Hover detection for BACK button
            for (size_t i = 0; i < settingsMenuItems_.size(); ++i) {
                if (settingsMenuItems_[i].getBackground().getGlobalBounds().contains(mousePos)) {
                    if (settingsHoveredIndex_ != static_cast<int>(i)) {
                        settingsMenuItems_[settingsHoveredIndex_].setHovered(false);
                        settingsHoveredIndex_ = static_cast<int>(i);
                        settingsMenuItems_[settingsHoveredIndex_].setHovered(true);
                    }
                    return;
                }
            }
        }
    }

    void Menu::handleMouseClick(sf::Vector2f mousePos) {
        if (state_ == MenuState::Closed) return;

        if (state_ == MenuState::MainMenu) {
            for (size_t i = 0; i < mainMenuItems_.size(); ++i) {
                if (mainMenuItems_[i].getBackground().getGlobalBounds().contains(mousePos)) {
                    selectMainItem(static_cast<int>(i));
                    return;
                }
            }
        }
        else if (state_ == MenuState::Settings) {
            // Clicking the BACK button
            for (auto& item : settingsMenuItems_) {
                if (item.getBackground().getGlobalBounds().contains(mousePos)) {
                    activateMainMenu();
                    return;
                }
            }
        }
    }

    // ── Private: execute a main-menu selection ────────────────────────────────

    void Menu::selectMainItem(int index) {
        const std::string& label = mainMenuItems_[index].getLabel();

        if (label == "RESUME") {
            close();                        // closing IS resuming
        }
        else if (label == "SETTINGS") {
            activateSettings();
        }
        else if (label == "QUIT") {
            if (onQuit) onQuit();
        }
    }

    // ── Update ───────────────────────────────────────────────────────────────

    void Menu::update(float deltaTime) {
        for (auto& item : mainMenuItems_)    item.update(deltaTime);
        for (auto& item : settingsMenuItems_) item.update(deltaTime);

        if (state_ == MenuState::Settings && settings_)
            settings_->update();
    }

    // ── Draw ──────────────────────────────────────────────────────────────────

    void Menu::draw() {
        if (state_ == MenuState::Closed) return;

        // Dim the game world behind the menu
        window_.draw(overlay_);

        if (state_ == MenuState::MainMenu) {
            // Panel background
            window_.draw(panel_);

            // "PAUSED" title centred above the items
            auto panelPos = panel_.getPosition();
            auto panelSize = panel_.getSize();
            auto tb = titleText_.getLocalBounds();
            titleText_.setPosition({
                panelPos.x + (panelSize.x - tb.size.x) / 2.f - tb.position.x,
                panelPos.y + 18.f
                });
            window_.draw(titleText_);

            // Menu items
            for (auto& item : mainMenuItems_)
                item.draw(window_, font_);
        }
        else if (state_ == MenuState::Settings) {
            // Let Settings draw its own panel (volume bars, title, hints)
            if (settings_) settings_->draw();

            // Draw the BACK button on top
            for (auto& item : settingsMenuItems_)
                item.draw(window_, font_);
        }
    }

    // ── Volume accessors (delegate to Settings) ───────────────────────────────

    float Menu::getMasterVolume() const {
        return settings_ ? settings_->getMasterVolume() : 0.f;
    }

    float Menu::getEffectVolume() const {
        return settings_ ? settings_->getEffectVolume() : 0.f;
    }

} // namespace corezone