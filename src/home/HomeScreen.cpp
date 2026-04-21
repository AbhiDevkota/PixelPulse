#include "HomeScreen.h"
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>

namespace corezone {

// --- Config class ---

const float Config::BG_COLOR = 0.0196f;        
const float Config::GRID_STRENGTH = 0.011f;
const int Config::INTRO_DURATION_MS = 5200;
const std::string Config::INTRO_INITIAL_TEXT = "Developed by GROUP X";


Menu::Menu() {              //<-----Menu Class----->
	//Menu Items
	addItem("UDD JETHA UDD");
	addItem("SNAKE");
	addItem("CHESS");
	addItem("CAR CHASE");
}

void Menu::addItem(const std::string& name) {
	items_.push_back(name);
}

void Menu::selectNext() {
	selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(items_.size());
}

void Menu::selectPrev() {
	selectedIndex_ = (selectedIndex_ - 1 + static_cast<int>(items_.size())) % static_cast<int>(items_.size());
}

const std::string& Menu::getSelectedName() const {
	return items_[selectedIndex_];
}

sf::FloatRect Menu::getBounds() const {
	return sf::FloatRect({0.0f, yPos_}, {480.0f, static_cast<float>(itemHeight_)});
}

// --- Effects class ---

void Effects::showIntro() {
    introVisible_ = true;
}

void Effects::hideIntro() {
    introVisible_ = false;
}

void Effects::triggerFlash() {
    flashActive_ = true;
}

bool Effects::isIntroVisible() const {
    return introVisible_;
}

// --- HomeScreen class ---

HomeScreen::HomeScreen(sf::RenderWindow& window, const std::string& fontPath,
                       const std::string& homeMusicPath, const std::string& selectSoundPath)
    : window_(window)
    , fontPath_(fontPath)
    , homeMusicPath_(homeMusicPath)
    , selectSoundPath_(selectSoundPath)
{
    menu_ = std::make_unique<Menu>();
    effects_ = std::make_unique<Effects>();
}

void HomeScreen::initialize() {
    // Load font
    if (!font_.openFromFile(fontPath_)) {
        // Try default path if file not found
        if (!font_.openFromFile("fonts/regular.ttf")) {
            // If still not found, continue (will use default rendering)
        }
    }

    setupMenu();
}

void HomeScreen::setupMenu() {
    // Menu items are already added in Menu constructor
    // Calculate positions
    itemHeight_ = 40;
    yPos_ = window_.getSize().y / 2.0f - (menu_->getItems().size() * itemHeight_) / 2.0f;
    menu_->setYPos(yPos_);
    menu_->setItemHeight(itemHeight_);
}

void HomeScreen::update(float deltaTime) {
    // Animate intro if visible
    animateIntro(deltaTime);

    // Check if intro should be hidden
    if (introFinished_ && introOpacity_ <= 0.0f) {
        introVisible_ = false;
    }
}

void HomeScreen::animateIntro(float deltaTime) {
    if (!introVisible_) return;

    animationTimer_ += deltaTime;

    // Fade out intro
    if (animationTimer_ > 2.0f) {
        introOpacity_ = 1.0f - (animationTimer_ - 2.0f) / 3.0f;
        if (introOpacity_ <= 0.0f) {
            introOpacity_ = 0.0f;
            introFinished_ = true;
        }
    }
}

void HomeScreen::draw() {
    // Draw background
    renderBackground();

    // Draw intro if visible
    if (introVisible_) {
        renderIntro();
    }

    // Draw flash effect
    renderFlash();

    // Draw title
    renderTitle();

    // Draw menu
    renderMenu();

    // Draw hint
    renderHint();
}

void HomeScreen::renderBackground() {
    auto windowSize = window_.getSize();

    // Draw grid
    sf::RectangleShape grid({static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)});
    grid.setPosition({0.0f, 0.0f});
    grid.setFillColor(sf::Color(
        1, 1, 1,
        static_cast<unsigned char>(Config::GRID_STRENGTH * 255))
    );
    window_.draw(grid);

    // Vertical grid lines
    for (float x = 0; x < windowSize.x; x += GRID_SPACING) {
        sf::RectangleShape line({1.0f, static_cast<float>(windowSize.y)});
        line.setPosition({x, 0.0f});
        line.setFillColor(sf::Color(1, 1, 1, 1));
        window_.draw(line);
    }

    // Horizontal grid lines
    for (float y = 0; y < windowSize.y; y += GRID_SPACING) {
        sf::RectangleShape line({static_cast<float>(windowSize.x), 1.0f});
        line.setPosition({0.0f, y});
        line.setFillColor(sf::Color(1, 1, 1, 1));
        window_.draw(line);
    }

    // Scanline effect
    for (float y = 0; y < windowSize.y; y += 2) {
        sf::RectangleShape scanline({static_cast<float>(windowSize.x), 2.0f});
        scanline.setPosition({0.0f, y});
        scanline.setFillColor(sf::Color(0, 0, 0, 13));
        window_.draw(scanline);
    }
}

void HomeScreen::renderIntro() {
    if (introVisible_) {
        sf::Text text("", font_);
        text.setCharacterSize(14);
        text.setLetterSpacing(2.0f);
        text.setFillColor(sf::Color(255, 255, 255, static_cast<unsigned char>(introOpacity_ * 255)));
        text.setString(Config::INTRO_INITIAL_TEXT);
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setPosition({
            (window_.getSize().x - textBounds.size.x) / 2.0f,
            (window_.getSize().y - textBounds.size.y) / 2.0f + 30.0f
        });

        window_.draw(text);
    }
}

void HomeScreen::renderFlash() {
    // Flash effect rendering (currently disabled)
    // Can be implemented later if needed
}

//void HomeScreen::renderFlash() {                              //(Lastai Annoying bhayo)
//    if (flashActive_) {
//        window_.resetFullscreen();
//        window_.setBackgroundColor(sf::Color(255, 255, 255));
//
//        // Flash animation
//        float flashAlpha = 0.85f;
//        float startTime = 0.0f;
//
//        if (flashActive_) {
//            flashSprite_.setColor(sf::Color(255, 255, 255, 255));
//
//            // Render a white flash overlay
//            sf::Sprite flashOverlay(window_.createTexture());
//            auto windowSize = window_.getSize();
//            flashOverlay.setTexture(sf::Texture(windowSize.x, windowSize.y));
//            flashOverlay.setPosition(0, 0);
//            flashOverlay.setColor(sf::Color(255, 255, 255));
//
//            window_.draw(flashOverlay);
//        }
//    } else {
//        window_.setFullscreen(sf::WindowHandle(sf::VideoMode::getDesktopMode()));
//    }
//}

void HomeScreen::renderTitle() {
    sf::Text title("CORE ZONE", font_);
    title.setCharacterSize(38);
    title.setLetterSpacing(0.5f);
    title.setFillColor(sf::Color(255, 255, 255));

    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setPosition({
        (window_.getSize().x - titleBounds.size.x) / 2.0f,
        (window_.getSize().y - titleBounds.size.y) / 2.0f - 65.0f
    });

    window_.draw(title);
}

void HomeScreen::renderMenu() {
    const auto& items = menu_->getItems();
    for (size_t i = 0; i < items.size(); ++i) {
        sf::Text text("", font_);
        text.setCharacterSize(20);
        text.setLetterSpacing(3.0f);
        text.setFillColor(sf::Color(255, 255, 255));

        // Add prefix for active item
        if (i == static_cast<size_t>(menu_->getSelectedIndex())) {
            text.setString("> " + items[i]);
            text.setFillColor(sf::Color(255, 255, 255));
        } else {
            text.setString(items[i]);
        }

        float yPos = menu_->getYPos() + (i * menu_->getItemHeight());
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setPosition({
            (window_.getSize().x - textBounds.size.x) / 2.0f,
            yPos
        });

        // Blink effect for active item
        if (i == static_cast<size_t>(menu_->getSelectedIndex())) {
            blinkTimer_ += 0.016f; // approximately 60fps
            float blinkCycle = std::fmod(blinkTimer_, 0.65f);
            if (blinkCycle < 0.325f) {
                text.setFillColor(sf::Color(255, 255, 255));
            } else {
                text.setFillColor(sf::Color(128, 128, 128));
            }
        }

        window_.draw(text);
    }
}

void HomeScreen::renderHint() {
    sf::Text hint("SELECT GAME TO START", font_);
    hint.setCharacterSize(11);
    hint.setLetterSpacing(1.0f);
    hint.setFillColor(sf::Color(80, 80, 80));

    sf::FloatRect hintBounds = hint.getLocalBounds();
    hint.setPosition({
        (window_.getSize().x - hintBounds.size.x) / 2.0f,
        window_.getSize().y - hintBounds.size.y - 15.0f
    });

    window_.draw(hint);
}

void HomeScreen::handleInput(const sf::Event& event) {
    if (event.is<sf::Event::KeyPressed>()) {
        if (const auto* keyPress = event.getIf<sf::Event::KeyPressed>()) {
            switch (keyPress->code) {
                case sf::Keyboard::Key::Down:
                case sf::Keyboard::Key::S:
                    menu_->selectNext();
                    playSelectSound();
                    break;
                case sf::Keyboard::Key::Up:
                case sf::Keyboard::Key::W:
                    menu_->selectPrev();
                    playSelectSound();
                    break;
                case sf::Keyboard::Key::Enter:
                case sf::Keyboard::Key::Space:
                    onGameSelected();
                    break;
                case sf::Keyboard::Key::Escape:
                    menu_->setSelectedIndex(0);
                    break;
                default:
                    break;
            }
        }
    }
}

void HomeScreen::handleResize(const sf::Event::Resized& event) {
    // Recalculate menu positions on window resize
    yPos_ = event.size.y / 2.0f - (menu_->getItems().size() * itemHeight_) / 2.0f;
    menu_->setYPos(yPos_);
}

void HomeScreen::playMusic() {
    homeMusic_ = std::make_unique<sf::Music>();
    if (homeMusic_->openFromFile(homeMusicPath_)) {
        homeMusic_->setLooping(true);
        homeMusic_->setVolume(50.0f);
        homeMusic_->play();
    }
}

void HomeScreen::playSelectSound() {
    selectSoundBuffer_ = std::make_unique<sf::SoundBuffer>();
    if (selectSoundBuffer_->loadFromFile(selectSoundPath_)) {
        selectSound_ = std::make_unique<sf::Sound>(*selectSoundBuffer_);
        selectSound_->play();
    }
}

void HomeScreen::onGameSelected() {
    std::string selected = menu_->getSelectedName();
    // Could show loading message here if we had hintText_ member
    // For now, just handle selection
}

void HomeScreen::cleanup() {
    if (homeMusic_) {
        homeMusic_->stop();
    }
    if (selectSound_) {
        selectSound_->stop();
    }
}

}
