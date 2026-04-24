#ifndef TUNGTUNG_TUNGTUNGGAME_H
#define TUNGTUNG_TUNGTUNGGAME_H

#include <SFML/Graphics.hpp>
#include <string>

#include "tungtungleague/GameSelectScreen.h"
#include "tungtungleague/baseball/BaseballGame.h"
#include "tungtungleague/cricket/CricketGame.h"

namespace tungtung {

class TungTungGame {
public:
    enum class Screen {
        GameSelect,
        Baseball,
        Cricket
    };

    TungTungGame(sf::RenderWindow& window, const std::string& fontPath);
    void run();

private:
    void wireCallbacks();

    sf::RenderWindow& window_;

    sf::Font font_;
    sf::Texture tungTungTexture_;

    Screen currentScreen_ = Screen::GameSelect;

    GameSelectScreen gameSelectScreen_;
    BaseballGame baseballGame_;
    CricketGame cricketGame_;
};

} // namespace tungtung

#endif // TUNGTUNG_TUNGTUNGGAME_H
