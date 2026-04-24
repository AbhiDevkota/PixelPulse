#ifndef TUNGTUNG_GAMESELECTSCREEN_H
#define TUNGTUNG_GAMESELECTSCREEN_H

#include <SFML/Graphics.hpp>
#include <functional>
#include "tungtungleague/common/AnimatedLine.h"

namespace tungtung {

class GameSelectScreen {
public:
    GameSelectScreen(sf::RenderWindow& window, sf::Font& font);

    void update(float dt);
    void draw();
    void handleInput(const sf::Event& event);
    void handleMouseMove(sf::Vector2f pos);
    void handleMouseClick(sf::Vector2f pos);

    std::function<void()> onBaseball;
    std::function<void()> onCricket;

private:
    struct Button {
        sf::RectangleShape box;
        sf::Text text;
        bool hovered = false;
    };

    void layout();
    void applyButtonStyle(Button& button);

    sf::RenderWindow& window_;
    sf::Font& font_;

    AnimatedLine animatedLine_;

    sf::RectangleShape background_;
    sf::Text titleText_;

    Button baseballButton_;
    Button cricketButton_;
};

} // namespace tungtung

#endif // TUNGTUNG_GAMESELECTSCREEN_H
