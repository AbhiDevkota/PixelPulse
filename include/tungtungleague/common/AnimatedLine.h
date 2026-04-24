#ifndef TUNGTUNG_ANIMATEDLINE_H
#define TUNGTUNG_ANIMATEDLINE_H

#include <SFML/Graphics.hpp>

namespace tungtung {

class AnimatedLine {
public:
    explicit AnimatedLine(sf::RenderWindow& window);

    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    void rebuildBand();

private:
    sf::RenderWindow& window_;
    sf::VertexArray band_;

    float lineHeight_ = 30.f;
    float speed_ = 200.f;
    float posY_ = -30.f;
};

} // namespace tungtung

#endif // TUNGTUNG_ANIMATEDLINE_H
