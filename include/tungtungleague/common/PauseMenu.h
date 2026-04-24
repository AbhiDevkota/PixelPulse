#ifndef TUNGTUNG_PAUSEMENU_H
#define TUNGTUNG_PAUSEMENU_H

#include <SFML/Graphics.hpp>
#include <functional>

namespace tungtung {

class PauseMenu {
public:
    PauseMenu(sf::RenderWindow& window, sf::Font& font);

    void update(float dt);
    void draw();
    void handleInput(const sf::Event& event);
    void handleMouseMove(sf::Vector2f pos);
    void handleMouseClick(sf::Vector2f pos);

    bool isPaused() const;
    void open();
    void close();

    std::function<void()> onResume;
    std::function<void()> onPlayAgain;
    std::function<void()> onExit;

private:
    struct Button {
        sf::RectangleShape box;
        sf::Text text;
        bool hovered = false;
    };

    void layout();
    void updateButtonStyle(Button& button);

    sf::RenderWindow& window_;
    sf::Font& font_;

    bool paused_ = false;

    // Top-left pause icon (❚❚)
    sf::RectangleShape pauseBarLeft_;
    sf::RectangleShape pauseBarRight_;
    sf::FloatRect pauseButtonBounds_;

    sf::RectangleShape overlay_;

    Button resumeButton_;
    Button playAgainButton_;
    Button exitButton_;
};

} // namespace tungtung

#endif // TUNGTUNG_PAUSEMENU_H
