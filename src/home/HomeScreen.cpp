#include "HomeScreen.h"
#include <iostream>
#include <cmath>
#include <cstdint>

namespace corezone {

    GameMenu::GameMenu() {
        items_ = { "UDD JETHA UDD", "SNAKE", "CHESS", "CAR CHASE" };
    }

    void GameMenu::selectNext() {
        selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(items_.size());
    }

    void GameMenu::selectPrev() {
        selectedIndex_ = (selectedIndex_ - 1 + static_cast<int>(items_.size())) % static_cast<int>(items_.size());
    }

    const std::string& GameMenu::getSelected() const {
        return items_[selectedIndex_];
    }

    HomeScreen::HomeScreen(sf::RenderWindow& window, const std::string& fontPath)
        : window_(window), gameMenu_(), menu_(window, font_), settings_(window, font_)
    {
        if (font_.openFromFile(fontPath)) {
            fontLoaded_ = true;
            std::cout << "Font loaded: " << fontPath << std::endl;
        }
        else {
            std::cerr << "Font not found: " << fontPath << std::endl;
        }

        if (iconFont_.openFromFile("fonts/DejaVuSans.ttf")) {
            std::cout << "Icon font loaded\n";
        }
        else {
            std::cerr << "Icon font not found (fonts/DejaVuSans.ttf)\n";
        }

        // ── AUDIO SETUP ───────────────────────────────────────────────────────

        // Background music — loops forever, starts when "Presenting You" fades out
        if (bgMusic_.openFromFile("audios/home/home_screen.wav")) {
            bgMusic_.setLooping(true);
            bgMusic_.setVolume(70.f);   // TWEAK: bg music volume (0–100)
            std::cout << "✓ BG music loaded\n";
        }
        else {
            std::cerr << "✗ BG music not found: audios/home/home_screen.wav\n";
        }

        // Navigate sound — plays on Up / Down / Escape, on top of bg music
        if (selectBuf_.loadFromFile("audios/home/select_game.wav")) {
            selectSnd_.setBuffer(selectBuf_);
            selectSnd_.setVolume(100.f);  // TWEAK: navigate sound volume (0–100)
            std::cout << "✓ Select sound loaded\n";
        }
        else {
            std::cerr << "✗ Select sound not found: audios/home/select_game.wav\n";
        }

        // Launch sound — plays on Enter / Space, on top of bg music
        if (launchBuf_.loadFromFile("audios/home/launch_game.wav")) {
            launchSnd_.setBuffer(launchBuf_);
            launchSnd_.setVolume(100.f);  // TWEAK: launch sound volume (0–100)
            std::cout << "✓ Launch sound loaded\n";
        }
        else {
            std::cerr << "✗ Launch sound not found: audios/home/launch_game.wav\n";
        }

        // ── MENU SETUP ────────────────────────────────────────────────────────
        menu_.onSettings = [this]() {
            settings_.show();
        };
        menu_.onQuit = [this]() {
            window_.close();
        };
        // ─────────────────────────────────────────────────────────────────────
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

                // ── Start bg music as "Presenting You" begins fading out ──────
                // TWEAK: the music kicks in at t=4.0s (when this else branch runs).
                // To start it earlier, move this block into the "time < 4.0f" branch.
                if (!bgMusicStarted_) {
                    bgMusic_.play();
                    bgMusicStarted_ = true;
                }
                // ─────────────────────────────────────────────────────────────

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

        // Update menu and settings
        menu_.update();
        settings_.update();
    }

    void HomeScreen::handleInput(const sf::Event& event) {
        if (state_ != State::Menu) return;

        // Handle pause menu first (ESC opens/closes it)
        if (event.is<sf::Event::KeyPressed>()) {
            const auto* key = event.getIf<sf::Event::KeyPressed>();
            if (key->code == sf::Keyboard::Key::Escape) {
                menu_.toggle();
                selectSnd_.play();
                return;
            }
        }

        // If menu is open, let it handle input
        if (menu_.getState() != Menu::MenuState::Closed) {
            menu_.handleInput(event);
            if (menu_.getState() == Menu::MenuState::MainMenu) {
                selectSnd_.play();
            }
            return;
        }

        // If settings is open, let it handle input
        if (settings_.isVisible()) {
            settings_.handleInput(event);
            return;
        }

        // Otherwise handle normal game menu input
        if (event.is<sf::Event::KeyPressed>()) {
            const auto* key = event.getIf<sf::Event::KeyPressed>();
            switch (key->code) {

            case sf::Keyboard::Key::Down:
            case sf::Keyboard::Key::S:
                gameMenu_.selectNext();
                selectSnd_.play();   // navigate beep on top of bg music
                break;

            case sf::Keyboard::Key::Up:
            case sf::Keyboard::Key::W:
                gameMenu_.selectPrev();
                selectSnd_.play();   // navigate beep on top of bg music
                break;

            case sf::Keyboard::Key::Enter:
            case sf::Keyboard::Key::Space:
                launchSnd_.play();   // launch sound on top of bg music
                startLoading();
                break;

            default: break;
            }
        }
    }

    void HomeScreen::handleMouseMove(sf::Vector2f mousePos) {
        if (state_ != State::Menu) return;
        menu_.handleMouseMove(mousePos);
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

        // Draw menu on top
        menu_.draw();

        // Draw settings on top
        settings_.draw();
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
        title.setLetterSpacing(3.0f);  // reduced from 8.0f — was too wide

        float flicker = std::sin(flickerClock_.getElapsedTime().asSeconds() * 8.0f) * 30.0f + 225.0f;
        std::uint8_t alpha = static_cast<std::uint8_t>(flicker);
        title.setFillColor(sf::Color(255, 255, 255, alpha));

        auto bounds = title.getLocalBounds();
        float cx = (static_cast<float>(window_.getSize().x) - bounds.size.x) / 2.0f;

        const float TITLE_Y = 360.0f;   //Increase garda down, Reduce garda UP
        title.setPosition({ cx, TITLE_Y });
        window_.draw(title);

        //Underline on the COREZONE
        float boxW = bounds.size.x + 40.0f;
        float boxH = 4.0f;
        sf::RectangleShape underline({ boxW, boxH });
        underline.setPosition({ cx - 20.0f, 356.0f + bounds.size.y + 10.0f });
        underline.setFillColor(sf::Color(255, 255, 255, alpha / 2));
        window_.draw(underline);
    }

    void HomeScreen::renderMenu() {
        if (!fontLoaded_) return;
        const auto& items = gameMenu_.getItems();
        float startY = static_cast<float>(window_.getSize().y) / 2.0f - 65.0f;   // moved up

        for (size_t i = 0; i < items.size(); ++i) {
            std::string display = items[i];
            if (static_cast<int>(i) == gameMenu_.getSelectedIndex()) {
                display = "> " + items[i] + " <";
            }

            sf::Text text(font_, display);
            text.setCharacterSize(26);
            text.setLetterSpacing(3.0f);

            if (static_cast<int>(i) == gameMenu_.getSelectedIndex()) {
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
        // + 65.0f lai - garda up, + garda down, 4*38.0f = 4 items, +16.0f = extra spacing
        float menuBottom = static_cast<float>(window_.getSize().y) / 2.0f + 65.0f + 4 * 38.0f + 16.0f;
        hint.setPosition({ x, menuBottom });

        window_.draw(hint);
    }

    void HomeScreen::renderControls() {
        if (!fontLoaded_) return;

        float baseY = static_cast<float>(window_.getSize().y) - 160.f;
        float keyX = 30.f;
        float labelX = 170.f;   // fixed label column, right of all keys

        // Row 1: up/down arrow keys + NAVIGATE label
        drawKey(window_, font_, sf::String(static_cast<char32_t>(0x2191)), { keyX,        baseY }, 50.f);
        drawKey(window_, font_, sf::String(static_cast<char32_t>(0x2193)), { keyX + 58.f, baseY }, 50.f);

        sf::Text nav(font_, "NAVIGATE", 18);
        nav.setFillColor(sf::Color(120, 120, 120));
        nav.setStyle(sf::Text::Italic);
        nav.setPosition({ labelX, baseY + 10.f });
        window_.draw(nav);

        // Row 2: ENTER + LAUNCH label
        drawKey(window_, font_, "ENTER", { keyX, baseY + 58.f }, 120.f);

        sf::Text launch(font_, "LAUNCH", 18);
        launch.setFillColor(sf::Color(120, 120, 120));
        launch.setStyle(sf::Text::Italic);
        launch.setPosition({ labelX, baseY + 68.f });
        window_.draw(launch);

        // Row 3: ESC + MENU label
        drawKey(window_, font_, "ESC", { keyX, baseY + 116.f }, 80.f);

        sf::Text menu(font_, "MENU", 18);
        menu.setFillColor(sf::Color(120, 120, 120));
        menu.setStyle(sf::Text::Italic);
        menu.setPosition({ labelX, baseY + 126.f });
        window_.draw(menu);
    }

    void HomeScreen::renderCredit() {
        if (!fontLoaded_) return;
        sf::Text credit(font_, "*Developed by ZONE BREACHER");
        credit.setCharacterSize(13);
        credit.setFillColor(sf::Color(100, 100, 100));
        credit.setStyle(sf::Text::Italic);

        auto bounds = credit.getLocalBounds();
        float x = static_cast<float>(window_.getSize().x) - bounds.size.x - 42.0f;
        credit.setPosition({ x, static_cast<float>(window_.getSize().y) - 38.0f });
        window_.draw(credit);
    }

    void HomeScreen::startLoading() {
        loadingName_ = gameMenu_.getSelected();
        loadingMode_ = true;
        flashActive_ = true;
        loadingClock_.restart();
    }

} // namespace corezone