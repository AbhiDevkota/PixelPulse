#ifndef TUNGTUNG_BASEBALLHUD_H
#define TUNGTUNG_BASEBALLHUD_H

#include <SFML/Graphics.hpp>

namespace tungtung {

class BaseballHUD {
public:
    BaseballHUD(sf::RenderWindow& window, sf::Font& font);

    void setInning(int inning);
    void setOuts(int outs);
    void setScores(int teamA, int teamB);

    void draw();

private:
    void refreshTexts();
    void layout();

    sf::RenderWindow& window_;
    sf::Font& font_;

    int inning_ = 1;
    int outs_ = 0;
    int teamAScore_ = 0;
    int teamBScore_ = 0;

    sf::Text inningOutsText_;
    sf::Text scoreText_;
};

} // namespace tungtung

#endif // TUNGTUNG_BASEBALLHUD_H
