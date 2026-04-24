#ifndef TUNGTUNG_BASEBALLFIELD_H
#define TUNGTUNG_BASEBALLFIELD_H

#include <SFML/Graphics.hpp>
#include <array>

namespace tungtung {

class BaseballField {
public:
    explicit BaseballField(sf::RenderWindow& window);

    void draw();

    sf::Vector2f getBasePosition(int base) const;   // 0=home, 1=first, 2=second, 3=third
    sf::Vector2f getPitcherPosition() const;
    sf::Vector2f getBatterPosition() const;
    sf::Vector2f getCatcherPosition() const;

private:
    void buildGeometry();

    sf::RenderWindow& window_;

    sf::RectangleShape background_;
    sf::CircleShape outfieldArc_;

    sf::ConvexShape diamond_;

    std::array<sf::RectangleShape, 4> baseSquares_;
    sf::ConvexShape homePlate_;
    sf::CircleShape pitcherMound_;

    sf::VertexArray foulLines_;

    std::array<sf::Vector2f, 4> basePositions_{};
    sf::Vector2f pitcherPosition_{};
    sf::Vector2f batterPosition_{};
    sf::Vector2f catcherPosition_{};
};

} // namespace tungtung

#endif // TUNGTUNG_BASEBALLFIELD_H
