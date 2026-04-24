#ifndef TUNGTUNG_PLAYER_H
#define TUNGTUNG_PLAYER_H

#include <SFML/Graphics.hpp>
#include <string>

namespace tungtung {

class Player {
public:
    Player(const sf::Texture& texture, const std::string& role);

    void setPosition(sf::Vector2f pos);
    void setFlipped(bool flipped);
    sf::Vector2f getPosition() const;
    void draw(sf::RenderWindow& window);

private:
    sf::Sprite sprite_;
    std::string role_;
    bool flipped_ = false;
};

} // namespace tungtung

#endif // TUNGTUNG_PLAYER_H
