#include "HomeScreen.h"
#include <iostream>
#include <cmath>
#include <cstdint>

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
        if (font_.openFromFile(fontPath)) {
            fontLoaded_ = true;
            std::cout << "✓ Font loaded: " << fontPath << std::endl;
        }
        else {
            std::cerr << "✗ Font not found: " << fontPath << std::endl;
        }

        if (iconFont_.openFromFile("fonts/DejaVuSans.ttf")) {
            std::cout << "✓ Icon font loaded\n";
        }
        else {
            std::cerr << "✗ Icon font not found (fonts/DejaVuSans.ttf)\n";
        }
    }

    void HomeScreen::initialize() {}

    void HomeScreen::update(float deltaTime) {
        if (state_ == State::Boot) {
            float time = introClock_.getElapsedTime().asSeconds();

            if (time < 1.8f) {
                introText_ = "Developed by ZONE BREACHER";
                introOpacity_ = 255.0f;
            }
            else if (time < 2.2f) {
                float progress = (time - 1.8f) / 0.4f;
                introOpacity_ = 255.0f * (1.0f - progress);
            }
            else if (time < 4.0f) {
                introText_ = "Presenting You";
                introOpacity_ = 255.0f;
            }
            else {
                introOpacity_ = std::max(0.0f, 255.0f - (time - 4.0f) * 180.0f);
                if (introOpacity_ <= 0.0f) {
                    state_ = State::Menu;
                    introVisible_ = false;
                }
            }
        }

        if (state_ == State::Menu && loadingMode_) {
            if (loadingClock_.getElapsedTime().asSeconds() > 0.4f) {
                dotCount_ = (dotCount_ + 1) % 4;
                loadingClock_.restart();
            }
        }
    }

    void HomeScreen::handleInput(const sf::Event& event) {
        if (state_ != State::Menu) return;

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

    void drawKey(sf::RenderWindow& window, sf::Font& font,
        const sf::String& label, sf::Vector2f pos, float width = 80.f)
    {
        sf::RectangleShape box({ width, 42.f });
        box.setPosition(pos);
        box.setFillColor(sf::Color::Transparent);
        box.setOutlineColor(sf::Color(100, 100, 100));
        box.setOutlineThickness(1.5f);

        sf::Text text(font, label, 20);
        text.setFillColor(sf::Color(160, 160, 160));
        text.setStyle(sf::Text::Italic);

        auto bounds = text.getLocalBounds();

        text.setPosition({
            pos.x + (width - bounds.size.x) / 2.f - bounds.position.x,
            pos.y + (42.f - bounds.size.y) / 2.f - bounds.position.y - 2.f
            });

        window.draw(box);
        window.draw(text);
    }

    void HomeScreen::draw() {
        renderBackground();

        if (state_ == State::Boot) {
            renderIntro();
        }
        else {
            renderTitle();
            renderMenu();
            renderHint();
            renderControls();
            renderCredit();
            if (flashActive_) renderFlash();
        }
    }

    void HomeScreen::renderBackground() {
        auto size = window_.getSize();

        sf::RectangleShape bg({ static_cast<float>(size.x), static_cast<float>(size.y) });
        bg.setFillColor(sf::Color(10, 10, 10));
        window_.draw(bg);

        if (state_ == State::Boot) return;

        // Scanlines
        for (float y = 0; y < static_cast<float>(size.y); y += 3.0f) {
            sf::RectangleShape line({ static_cast<float>(size.x), 1.0f });
            line.setPosition({ 0.0f, y });
            line.setFillColor(sf::Color(0, 0, 0, 45));
            window_.draw(line);
        }

        // Moving CRT beam (with gradient falloff)
        float beamY = std::fmod(
            beamClock_.getElapsedTime().asSeconds() * 90.0f,
            static_cast<float>(size.y) + 300.0f
        ) - 300.0f;

        const float beamHeight = 240.0f;                        //Background Beam Height. Badayo bhane width badxa
        const float center = beamHeight / 2.0f;

        sf::VertexArray beam(sf::PrimitiveType::TriangleStrip);

        float time = beamClock_.getElapsedTime().asSeconds();
        const int groupSize = 5;

        for (int i = 0; i <= static_cast<int>(beamHeight); i += groupSize)
        {
            // Distance from center (0 → center, 1 → edges)
            float dist = std::abs(i - center) / center;

            // Softer falloff
            float baseIntensity = std::pow(1.0f - dist, 1.8f);

            // Slightly reduce center brightness
            baseIntensity *= (0.85f + 0.15f * dist);

            // Flicker
            float flicker =
                0.92f +
                0.06f * std::sin(time * 18.0f + i * 0.15f) +
                0.04f * std::sin(time * 3.5f);


            for (int j = 0; j < groupSize; ++j) //Draw Multiple Lines
            {
                float y = beamY + i + j;

                // Slight variation inside group (top a bit brighter)
                float localFactor = 1.0f - (j / static_cast<float>(groupSize)) * 0.15f;

                float intensity = baseIntensity * localFactor;

                std::uint8_t alpha = static_cast<std::uint8_t>(intensity * 15.0f * flicker);

                sf::Color col(255, 255, 255, alpha);

                beam.append(sf::Vertex({ 0.0f, y }, col));
                beam.append(sf::Vertex({ static_cast<float>(size.x), y }, col));
            }
        }

        window_.draw(beam);
    }

    void HomeScreen::renderIntro() {
        if (!fontLoaded_) return;
        sf::Text text(font_, introText_);
        text.setCharacterSize(24);
        text.setLetterSpacing(2.0f);
        text.setFillColor(sf::Color(255, 255, 255, static_cast<unsigned char>(introOpacity_)));

        if (introText_ == "Presenting You") {
            text.setStyle(sf::Text::Italic);
        }

        auto bounds = text.getLocalBounds();
        float x = (static_cast<float>(window_.getSize().x) - bounds.size.x) / 2.0f;
        float y = (static_cast<float>(window_.getSize().y) - bounds.size.y) / 2.0f;
        text.setPosition({ x, y });
        window_.draw(text);
    }

    void HomeScreen::renderFlash() {
        sf::RectangleShape flash({ static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y) });
        flash.setFillColor(sf::Color(255, 255, 255, 200));
        window_.draw(flash);
        flashActive_ = false;
    }

    void HomeScreen::renderTitle() {
        if (!fontLoaded_) return;
        sf::Text title(font_, "CORE ZONE");
        title.setCharacterSize(58);
        title.setLetterSpacing(8.0f);

        float flicker = std::sin(flickerClock_.getElapsedTime().asSeconds() * 8.0f) * 30.0f + 225.0f;
        title.setFillColor(sf::Color(255, 255, 255, static_cast<unsigned char>(flicker)));

        auto bounds = title.getLocalBounds();
        float x = (static_cast<float>(window_.getSize().x) - bounds.size.x) / 2.0f;
        title.setPosition({ x, 110.0f });
        window_.draw(title);
    }

    void HomeScreen::renderMenu() {
        if (!fontLoaded_) return;
        const auto& items = menu_.getItems();
        float startY = static_cast<float>(window_.getSize().y) / 2.0f - 65.0f;   // moved up

        for (size_t i = 0; i < items.size(); ++i) {
            std::string display = items[i];
            if (static_cast<int>(i) == menu_.getSelectedIndex()) {
                display = "> " + items[i] + " <";
            }

            sf::Text text(font_, display);
            text.setCharacterSize(26);
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
            float y = startY + static_cast<float>(i) * 38.0f;   // less space

            text.setPosition({ x, y });
            window_.draw(text);
        }
    }

    void HomeScreen::renderHint() {
        if (!fontLoaded_) return;

        sf::Text hint(font_, "", 13);
        hint.setLetterSpacing(1.0f);
        hint.setFillColor(sf::Color(160, 160, 160));

        if (loadingMode_) {
            std::string loadingStr = ">> LOADING  " + loadingName_ + "  " + std::string(dotCount_, '.');
            hint.setString(loadingStr);
            hint.setStyle(sf::Text::Italic);
        }
        else {
            hint.setString("SELECT GAME TO START");
            hint.setStyle(sf::Text::Regular);
        }

        auto bounds = hint.getLocalBounds();
        float x = (static_cast<float>(window_.getSize().x) - bounds.size.x) / 2.0f;
        hint.setPosition({ x, static_cast<float>(window_.getSize().y) - 105.0f });

        window_.draw(hint);
    }

    void HomeScreen::renderControls() {
        if (!fontLoaded_) return;

        float baseY = static_cast<float>(window_.getSize().y) - 120.f;

        sf::String up = sf::String(static_cast<char32_t>(0x2191));
        sf::String down = sf::String(static_cast<char32_t>(0x2193));

        sf::Text nav(font_, "NAVIGATE", 20);
        nav.setFillColor(sf::Color(120, 120, 120));
        nav.setStyle(sf::Text::Italic);
        nav.setPosition({ 210.f, baseY + 8.f });
        window_.draw(nav);

        drawKey(window_, font_, "ENTER", { 38.f, baseY + 50.f }, 130.f);

        sf::Text launch(font_, "LAUNCH", 20);
        launch.setFillColor(sf::Color(120, 120, 120));
        launch.setStyle(sf::Text::Italic);
        launch.setPosition({ 180.f, baseY + 58.f });
        window_.draw(launch);

        drawKey(window_, font_, "ESC", { 38.f, baseY + 100.f }, 100.f);

        sf::Text menu(font_, "MENU", 20);
        menu.setFillColor(sf::Color(120, 120, 120));
        menu.setStyle(sf::Text::Italic);
        menu.setPosition({ 150.f, baseY + 108.f });
        window_.draw(menu);
    }

    void HomeScreen::renderCredit() {
        if (!fontLoaded_) return;
        sf::Text credit(font_, "*Developed by ZONE BREACHER");
        credit.setCharacterSize(13);                  // bigger as requested
        credit.setFillColor(sf::Color(100, 100, 100));
        credit.setStyle(sf::Text::Italic);


        auto bounds = credit.getLocalBounds();
        float x = static_cast<float>(window_.getSize().x) - bounds.size.x - 42.0f;
        credit.setPosition({ x, static_cast<float>(window_.getSize().y) - 38.0f });
        window_.draw(credit);
    }

    void HomeScreen::startLoading() {
        loadingName_ = menu_.getSelected();
        loadingMode_ = true;
        flashActive_ = true;
        loadingClock_.restart();
    }

} // namespace corezone