#ifndef TUNGTUNG_CRICKETFIELD_H
#define TUNGTUNG_CRICKETFIELD_H

#include <SFML/Graphics.hpp>
#include <array>

namespace tungtung {

class CricketField {
public:
    explicit CricketField(sf::RenderWindow& window);

    void draw();

    sf::Vector2f getBatterPosition() const;
    sf::Vector2f getBowlerStartPosition() const;
    sf::Vector2f getBowlerCreasePosition() const;
    sf::Vector2f getKeeperPosition() const;
    sf::Vector2f getStumpCenter(int end) const; // 0=batter end, 1=bowler end

private:
    void buildGeometry();

    sf::RenderWindow& window_;

    sf::RectangleShape outfield_;
    sf::CircleShape oval_;
    sf::RectangleShape pitch_;

    std::array<sf::RectangleShape, 6> verticalStumps_;
    std::array<sf::RectangleShape, 4> bails_;

    sf::VertexArray creaseLines_;

    sf::Vector2f batterStumpCenter_{};
    sf::Vector2f bowlerStumpCenter_{};
    sf::Vector2f batterPosition_{};
    sf::Vector2f bowlerStartPosition_{};
    sf::Vector2f bowlerCreasePosition_{};
    sf::Vector2f keeperPosition_{};
};

} // namespace tungtung

#endif // TUNGTUNG_CRICKETFIELD_H
