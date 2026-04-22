#include "HomeScreen.h"
#include <iostream>
#include <cmath>

namespace corezone {

    Menu::Menu() {
        items_ = { "UDD JETHA UDD", "SNAKE", "CHESS", "CAR CHASE" };
    }

    void Menu::selectNext() {
        selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(items_.size());
    }

    void Menu::selectPrev() {
        selectedIndex_ = (selectedIndex_ - 1 + static_cast<int>(items_.size())) % static_cast<int>(items_.size());
    }

    const std::string& Menu::getSelected() const {
        return items_[selectedIndex_];
    }

    HomeScreen::HomeScreen(sf::RenderWindow& window, const std::string& fontPath)
        : window_(window)
    {
        // Try the path passed from main.cpp
        if (font_.openFromFile(fontPath)) {
            fontLoaded_ = true;
            std::cout << "✓ Font loaded successfully: " << fontPath << std::endl;
        }
        // Extra fallback for regular.ttf
        else if (font_.openFromFile("fonts/regular.ttf")) {
            fontLoaded_ = true;
            std::cout << "✓ Font loaded from fonts/regular.ttf" << std::endl;
        }
        else {
            std::cerr << "✗ FONT NOT FOUND!\n"
                << "   Your font file is named regular.ttf\n"
                << "   Make sure it is in fonts/ folder\n" << std::endl;
        }
    }

    void HomeScreen::initialize() {
        // Nothing extra needed
    }

    void HomeScreen::update(float deltaTime) {
        if (introVisible_) {
            float time = introClock_.getElapsedTime().asSeconds();
            if (time > 2.0f && introText_ == "Developed by GROUP X") {
                introText_ = "Presenting You";
            }
            if (time > 4.2f) {
                introOpacity_ = std::max(0.0f, introOpacity_ - deltaTime * 180.0f);
                if (introOpacity_ <= 0.0f) introVisible_ = false;
            }
        }

        if (loadingMode_) {
            if (loadingClock_.getElapsedTime().asSeconds() > 0.4f) {
                dotCount_ = (dotCount_ + 1) % 4;
                loadingClock_.restart();
            }
        }
    }

    void HomeScreen::handleInput(const sf::Event& event) {
        if (event.is<sf::Event::KeyPressed>()) {
            const auto* key = event.getIf<sf::Event::KeyPressed>();
            switch (key->code) {
            case sf::Keyboard::Key::Down: case sf::Keyboard::Key::S: menu_.selectNext(); break;
            case sf::Keyboard::Key::Up:   case sf::Keyboard::Key::W: menu_.selectPrev(); break;
            case sf::Keyboard::Key::Enter: case sf::Keyboard::Key::Space: startLoading(); break;
            case sf::Keyboard::Key::Escape: menu_.selectPrev(); break;
            default: break;
            }
        }
    }

    void HomeScreen::draw() {
        renderBackground();
        if (introVisible_) renderIntro();
        if (flashActive_) renderFlash();

        renderTitle();
        renderMenu();
        renderHint();
        renderControls();
        renderCredit();
    }

    void HomeScreen::renderBackground() {
        auto size = window_.getSize();
        sf::RectangleShape bg({ static_cast<float>(size.x), static_cast<float>(size.y) });
        bg.setFillColor(sf::Color(10, 10, 10));
        window_.draw(bg);

        // Grid
        for (float x = 0; x < static_cast<float>(size.x); x += 28.0f) {
            sf::RectangleShape line({ 1.0f, static_cast<float>(size.y) });
            line.setPosition({ x, 0.0f });
            line.setFillColor(sf::Color(255, 255, 255, 28));
            window_.draw(line);
        }
        for (float y = 0; y < static_cast<float>(size.y); y += 28.0f) {
            sf::RectangleShape line({ static_cast<float>(size.x), 1.0f });
            line.setPosition({ 0.0f, y });
            line.setFillColor(sf::Color(255, 255, 255, 28));
            window_.draw(line);
        }

        // Scanlines
        for (float y = 0; y < static_cast<float>(size.y); y += 3.0f) {
            sf::RectangleShape line({ static_cast<float>(size.x), 1.0f });
            line.setPosition({ 0.0f, y });
            line.setFillColor(sf::Color(0, 0, 0, 45));
            window_.draw(line);
        }
    }

    void HomeScreen::renderIntro() {
        if (!fontLoaded_) return;
        sf::Text text(font_, introText_);
        text.setCharacterSize(22);
        text.setLetterSpacing(2.0f);
        text.setFillColor(sf::Color(255, 255, 255, static_cast<unsigned char>(introOpacity_)));

        auto bounds = text.getLocalBounds();
        float x = (static_cast<float>(window_.getSize().x) - bounds.size.x) / 2.0f;
        float y = (static_cast<float>(window_.getSize().y) - bounds.size.y) / 2.0f;

        text.setPosition({ x, y });
        window_.draw(text);
    }

    void HomeScreen::renderFlash() {
        sf::RectangleShape flash({ static_cast<float>(window_.getSize().x),
                                  static_cast<float>(window_.getSize().y) });
        flash.setFillColor(sf::Color(255, 255, 255, 200));
        window_.draw(flash);
        flashActive_ = false;
    }

    void HomeScreen::renderTitle() {
        if (!fontLoaded_) return;
        sf::Text title(font_, "CORE ZONE");
        title.setCharacterSize(52);
        title.setLetterSpacing(6.0f);

        float flicker = std::sin(flickerClock_.getElapsedTime().asSeconds() * 8.0f) * 30.0f + 225.0f;
        title.setFillColor(sf::Color(255, 255, 255, static_cast<unsigned char>(flicker)));

        auto bounds = title.getLocalBounds();
        float x = (static_cast<float>(window_.getSize().x) - bounds.size.x) / 2.0f;
        title.setPosition({ x, 80.0f });
        window_.draw(title);
    }

    void HomeScreen::renderMenu() {
        if (!fontLoaded_) return;
        const auto& items = menu_.getItems();
        float startY = static_cast<float>(window_.getSize().y) / 2.0f - 40.0f;

        for (size_t i = 0; i < items.size(); ++i) {
            std::string display = items[i];
            if (static_cast<int>(i) == menu_.getSelectedIndex()) {
                display = "> " + items[i] + " <";
            }

            sf::Text text(font_, display);
            text.setCharacterSize(20);
            text.setLetterSpacing(3.0f);

            if (static_cast<int>(i) == menu_.getSelectedIndex()) {
                float blink = std::sin(blinkClock_.getElapsedTime().asSeconds() * 8.0f);
                text.setFillColor(blink > 0.0f ? sf::Color(255, 255, 255) : sf::Color(180, 180, 180));
            }
            else {
                text.setFillColor(sf::Color(255, 255, 255));
            }

            auto bounds = text.getLocalBounds();
            float x = (static_cast<float>(window_.getSize().x) - bounds.size.x) / 2.0f;
            float y = startY + static_cast<float>(i) * 42.0f;

            text.setPosition({ x, y });
            window_.draw(text);
        }
    }

    void HomeScreen::renderHint() {
        if (!fontLoaded_) return;
        std::string hintStr = loadingMode_
            ? ">> LOADING  " + loadingName_ + "  " + std::string(dotCount_, '.')
            : "PRESS ENTER TO START";

        sf::Text hint(font_, hintStr);
        hint.setCharacterSize(11);
        hint.setLetterSpacing(1.0f);
        hint.setFillColor(sf::Color(120, 120, 120));

        auto bounds = hint.getLocalBounds();
        float x = (static_cast<float>(window_.getSize().x) - bounds.size.x) / 2.0f;
        hint.setPosition({ x, static_cast<float>(window_.getSize().y) - 65.0f });
        window_.draw(hint);
    }

    void HomeScreen::renderControls() {
        if (!fontLoaded_) return;
        sf::Text ctrl(font_, "↑ ↓ NAVIGATE\nENTER LAUNCH\nESC MENU");
        ctrl.setCharacterSize(9);
        ctrl.setFillColor(sf::Color(80, 80, 80));
        ctrl.setPosition({ 32.0f, static_cast<float>(window_.getSize().y) - 70.0f });
        window_.draw(ctrl);
    }

    void HomeScreen::renderCredit() {
        if (!fontLoaded_) return;
        sf::Text credit(font_, "*Developed by Group X");
        credit.setCharacterSize(11);
        credit.setFillColor(sf::Color(80, 80, 80));

        auto bounds = credit.getLocalBounds();
        float x = static_cast<float>(window_.getSize().x) - bounds.size.x - 40.0f;
        credit.setPosition({ x, static_cast<float>(window_.getSize().y) - 35.0f });
        window_.draw(credit);
    }

    void HomeScreen::startLoading() {
        loadingName_ = menu_.getSelected();
        loadingMode_ = true;
        flashActive_ = true;
        loadingClock_.restart();
    }

} // namespace corezone