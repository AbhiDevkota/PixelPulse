#ifndef TUNGTUNG_CRICKETHUD_H
#define TUNGTUNG_CRICKETHUD_H

#include <SFML/Graphics.hpp>

namespace tungtung {

class CricketHUD {
public:
    CricketHUD(sf::RenderWindow& window, sf::Font& font);

    void setScore(int runs, int wickets);
    void setBalls(int totalBalls);
    void setTarget(int target); // -1 hides target

    void draw();

private:
    void refreshTexts();
    void layout();

    sf::RenderWindow& window_;
    sf::Font& font_;

    int runs_ = 0;
    int wickets_ = 0;
    int balls_ = 0;
    int target_ = -1;

    sf::Text scoreOversText_;
    sf::Text targetText_;
};

} // namespace tungtung

#endif // TUNGTUNG_CRICKETHUD_H
