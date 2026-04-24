#include "tungtungleague/common/PauseMenu.h"

namespace tungtung {

PauseMenu::PauseMenu(sf::RenderWindow& window, sf::Font& font)
    : window_(window),
      font_(font),
      resumeButton_{ sf::RectangleShape({ 280.f, 64.f }), sf::Text(font_, "RESUME", 28), false },
      playAgainButton_{ sf::RectangleShape({ 280.f, 64.f }), sf::Text(font_, "PLAY AGAIN", 28), false },
      exitButton_{ sf::RectangleShape({ 280.f, 64.f }), sf::Text(font_, "EXIT", 28), false } {
    pauseBarLeft_.setSize({ 8.f, 28.f });
    pauseBarRight_.setSize({ 8.f, 28.f });
    pauseBarLeft_.setFillColor(sf::Color::White);
    pauseBarRight_.setFillColor(sf::Color::White);

    overlay_.setFillColor(sf::Color(0, 0, 0, 170));

    resumeButton_.box.setOutlineThickness(2.f);
    playAgainButton_.box.setOutlineThickness(2.f);
    exitButton_.box.setOutlineThickness(2.f);

    layout();
    updateButtonStyle(resumeButton_);
    updateButtonStyle(playAgainButton_);
    updateButtonStyle(exitButton_);
}

void PauseMenu::update(float) {
    // No time-based animation required for this menu.
}

void PauseMenu::draw() {
    // Pause icon is always shown at top-left.
    window_.draw(pauseBarLeft_);
    window_.draw(pauseBarRight_);

    if (!paused_) {
        return;
    }

    window_.draw(overlay_);

    window_.draw(resumeButton_.box);
    window_.draw(resumeButton_.text);

    window_.draw(playAgainButton_.box);
    window_.draw(playAgainButton_.text);

    window_.draw(exitButton_.box);
    window_.draw(exitButton_.text);
}

void PauseMenu::handleInput(const sf::Event& event) {
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

void PauseMenu::handleMouseMove(sf::Vector2f pos) {
    if (!paused_) {
        return;
    }

    resumeButton_.hovered = resumeButton_.box.getGlobalBounds().contains(pos);
    playAgainButton_.hovered = playAgainButton_.box.getGlobalBounds().contains(pos);
    exitButton_.hovered = exitButton_.box.getGlobalBounds().contains(pos);

    updateButtonStyle(resumeButton_);
    updateButtonStyle(playAgainButton_);
    updateButtonStyle(exitButton_);
}

void PauseMenu::handleMouseClick(sf::Vector2f pos) {
    if (!paused_) {
        if (pauseButtonBounds_.contains(pos)) {
            open();
        }
        return;
    }

    if (resumeButton_.box.getGlobalBounds().contains(pos)) {
        close();
        if (onResume) {
            onResume();
        }
        return;
    }

    if (playAgainButton_.box.getGlobalBounds().contains(pos)) {
        close();
        if (onPlayAgain) {
            onPlayAgain();
        }
        return;
    }

    if (exitButton_.box.getGlobalBounds().contains(pos)) {
        close();
        if (onExit) {
            onExit();
        }
    }
}

bool PauseMenu::isPaused() const {
    return paused_;
}

void PauseMenu::open() {
    paused_ = true;
}

void PauseMenu::close() {
    paused_ = false;

    resumeButton_.hovered = false;
    playAgainButton_.hovered = false;
    exitButton_.hovered = false;

    updateButtonStyle(resumeButton_);
    updateButtonStyle(playAgainButton_);
    updateButtonStyle(exitButton_);
}

void PauseMenu::layout() {
    const sf::Vector2u size = window_.getSize();
    const float windowWidth = static_cast<float>(size.x);
    const float windowHeight = static_cast<float>(size.y);

    overlay_.setSize({ windowWidth, windowHeight });

    // Pause icon at (20, 20)
    pauseBarLeft_.setPosition({ 20.f, 20.f });
    pauseBarRight_.setPosition({ 34.f, 20.f });
    pauseButtonBounds_ = sf::FloatRect({ 16.f, 16.f }, { 34.f, 36.f });

    const float spacing = 20.f;
    const float buttonWidth = resumeButton_.box.getSize().x;
    const float buttonHeight = resumeButton_.box.getSize().y;
    const float totalHeight = (buttonHeight * 3.f) + (spacing * 2.f);

    const float x = (windowWidth - buttonWidth) * 0.5f;
    const float startY = (windowHeight - totalHeight) * 0.5f;

    resumeButton_.box.setPosition({ x, startY });
    playAgainButton_.box.setPosition({ x, startY + buttonHeight + spacing });
    exitButton_.box.setPosition({ x, startY + (buttonHeight + spacing) * 2.f });

    auto centerText = [](Button& button) {
        const auto bounds = button.text.getLocalBounds();
        button.text.setPosition({
            button.box.getPosition().x + (button.box.getSize().x - bounds.size.x) * 0.5f - bounds.position.x,
            button.box.getPosition().y + (button.box.getSize().y - bounds.size.y) * 0.5f - bounds.position.y - 2.f
        });
    };

    centerText(resumeButton_);
    centerText(playAgainButton_);
    centerText(exitButton_);
}

void PauseMenu::updateButtonStyle(Button& button) {
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
