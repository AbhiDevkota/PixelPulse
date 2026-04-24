#include "tungtungleague/TungTungGame.h"

#include <optional>

namespace tungtung {

TungTungGame::TungTungGame(sf::RenderWindow& window, const std::string& fontPath)
    : window_(window),
      gameSelectScreen_(window_, font_),
      baseballGame_(window_, font_, tungTungTexture_),
      cricketGame_(window_, font_, tungTungTexture_) {
    font_.openFromFile(fontPath);
    tungTungTexture_.loadFromFile("assets/tungtungleague/tung_tung.png");

    wireCallbacks();
}

void TungTungGame::run() {
    sf::Clock clock;

    while (window_.isOpen()) {
        while (const std::optional<sf::Event> event = window_.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window_.close();

            switch (currentScreen_) {
            case Screen::GameSelect:
                gameSelectScreen_.handleInput(*event);
                break;
            case Screen::Baseball:
                baseballGame_.handleInput(*event);
                break;
            case Screen::Cricket:
                cricketGame_.handleInput(*event);
                break;
            }

            if (event->is<sf::Event::MouseMoved>()) {
                const auto* mouse = event->getIf<sf::Event::MouseMoved>();
                const sf::Vector2f mousePos = window_.mapPixelToCoords({ mouse->position.x, mouse->position.y });

                switch (currentScreen_) {
                case Screen::GameSelect:
                    gameSelectScreen_.handleMouseMove(mousePos);
                    break;
                case Screen::Baseball:
                    baseballGame_.handleMouseMove(mousePos);
                    break;
                case Screen::Cricket:
                    cricketGame_.handleMouseMove(mousePos);
                    break;
                }
            }

            if (event->is<sf::Event::MouseButtonPressed>()) {
                const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>();
                if (mouse->button == sf::Mouse::Button::Left) {
                    const sf::Vector2f mousePos = window_.mapPixelToCoords({ mouse->position.x, mouse->position.y });

                    switch (currentScreen_) {
                    case Screen::GameSelect:
                        gameSelectScreen_.handleMouseClick(mousePos);
                        break;
                    case Screen::Baseball:
                        baseballGame_.handleMouseClick(mousePos);
                        break;
                    case Screen::Cricket:
                        cricketGame_.handleMouseClick(mousePos);
                        break;
                    }
                }
            }
        }

        const float dt = clock.restart().asSeconds();

        switch (currentScreen_) {
        case Screen::GameSelect:
            gameSelectScreen_.update(dt);
            break;
        case Screen::Baseball:
            baseballGame_.update(dt);
            break;
        case Screen::Cricket:
            cricketGame_.update(dt);
            break;
        }

        window_.clear();

        switch (currentScreen_) {
        case Screen::GameSelect:
            gameSelectScreen_.draw();
            break;
        case Screen::Baseball:
            baseballGame_.draw();
            break;
        case Screen::Cricket:
            cricketGame_.draw();
            break;
        }

        window_.display();
    }
}

void TungTungGame::wireCallbacks() {
    gameSelectScreen_.onBaseball = [this]() {
        currentScreen_ = Screen::Baseball;
    };

    gameSelectScreen_.onCricket = [this]() {
        currentScreen_ = Screen::Cricket;
    };

    baseballGame_.onExit = [this]() {
        currentScreen_ = Screen::GameSelect;
    };

    cricketGame_.onExit = [this]() {
        currentScreen_ = Screen::GameSelect;
    };
}

} // namespace tungtung
