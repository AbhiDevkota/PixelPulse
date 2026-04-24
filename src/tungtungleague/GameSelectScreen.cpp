#include "tungtungleague/GameSelectScreen.h"

namespace tungtung {

GameSelectScreen::GameSelectScreen(sf::RenderWindow& window, sf::Font& font)
    : window_(window),
      font_(font),
      animatedLine_(window),
      titleText_(font_, "Choose the game you wanna play", 32),
      baseballButton_{ sf::RectangleShape({ 260.f, 72.f }), sf::Text(font_, "BASEBALL", 30), false },
      cricketButton_{ sf::RectangleShape({ 260.f, 72.f }), sf::Text(font_, "CRICKET", 30), false } {
    background_.setSize({ static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y) });
    background_.setFillColor(sf::Color::Black);

    titleText_.setFillColor(sf::Color::White);

    baseballButton_.box.setOutlineThickness(2.f);
    cricketButton_.box.setOutlineThickness(2.f);

    baseballButton_.text.setFillColor(sf::Color::White);
    cricketButton_.text.setFillColor(sf::Color::White);

    layout();
    applyButtonStyle(baseballButton_);
    applyButtonStyle(cricketButton_);
}

void GameSelectScreen::update(float dt) {
    animatedLine_.update(dt);
}

void GameSelectScreen::draw() {
    window_.draw(background_);
    animatedLine_.draw(window_);

    window_.draw(titleText_);
    window_.draw(baseballButton_.box);
    window_.draw(baseballButton_.text);
    window_.draw(cricketButton_.box);
    window_.draw(cricketButton_.text);
}

void GameSelectScreen::handleInput(const sf::Event& event) {
    if (event.is<sf::Event::MouseMoved>()) {
        const auto* mouse = event.getIf<sf::Event::MouseMoved>();
        handleMouseMove(window_.mapPixelToCoords({ mouse->position.x, mouse->position.y }));
    }
    else if (event.is<sf::Event::MouseButtonPressed>()) {
        const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>();
        if (mouse->button == sf::Mouse::Button::Left) {
            handleMouseClick(window_.mapPixelToCoords({ mouse->position.x, mouse->position.y }));
        }
    }
}

void GameSelectScreen::handleMouseMove(sf::Vector2f pos) {
    baseballButton_.hovered = baseballButton_.box.getGlobalBounds().contains(pos);
    cricketButton_.hovered = cricketButton_.box.getGlobalBounds().contains(pos);

    applyButtonStyle(baseballButton_);
    applyButtonStyle(cricketButton_);
}

void GameSelectScreen::handleMouseClick(sf::Vector2f pos) {
    if (baseballButton_.box.getGlobalBounds().contains(pos)) {
        if (onBaseball) {
            onBaseball();
        }
        return;
    }

    if (cricketButton_.box.getGlobalBounds().contains(pos)) {
        if (onCricket) {
            onCricket();
        }
    }
}

void GameSelectScreen::layout() {
    const float windowWidth = static_cast<float>(window_.getSize().x);
    const float windowHeight = static_cast<float>(window_.getSize().y);

    auto titleBounds = titleText_.getLocalBounds();
    titleText_.setPosition({
        (windowWidth - titleBounds.size.x) * 0.5f - titleBounds.position.x,
        (windowHeight * 0.5f) - 120.f - titleBounds.position.y
    });

    const float spacing = 40.f;
    const float totalWidth = baseballButton_.box.getSize().x + cricketButton_.box.getSize().x + spacing;
    const float startX = (windowWidth - totalWidth) * 0.5f;
    const float y = (windowHeight * 0.5f) - 10.f;

    baseballButton_.box.setPosition({ startX, y });
    cricketButton_.box.setPosition({ startX + baseballButton_.box.getSize().x + spacing, y });

    auto baseballTextBounds = baseballButton_.text.getLocalBounds();
    baseballButton_.text.setPosition({
        baseballButton_.box.getPosition().x + (baseballButton_.box.getSize().x - baseballTextBounds.size.x) * 0.5f - baseballTextBounds.position.x,
        baseballButton_.box.getPosition().y + (baseballButton_.box.getSize().y - baseballTextBounds.size.y) * 0.5f - baseballTextBounds.position.y - 2.f
    });

    auto cricketTextBounds = cricketButton_.text.getLocalBounds();
    cricketButton_.text.setPosition({
        cricketButton_.box.getPosition().x + (cricketButton_.box.getSize().x - cricketTextBounds.size.x) * 0.5f - cricketTextBounds.position.x,
        cricketButton_.box.getPosition().y + (cricketButton_.box.getSize().y - cricketTextBounds.size.y) * 0.5f - cricketTextBounds.position.y - 2.f
    });
}

void GameSelectScreen::applyButtonStyle(Button& button) {
    if (button.hovered) {
        button.box.setFillColor(sf::Color::White);
        button.box.setOutlineColor(sf::Color::White);
        button.text.setFillColor(sf::Color::Black);
    }
    else {
        button.box.setFillColor(sf::Color::Black);
        button.box.setOutlineColor(sf::Color::White);
        button.text.setFillColor(sf::Color::White);
    }
}

} // namespace tungtung
