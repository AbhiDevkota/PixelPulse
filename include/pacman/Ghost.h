#ifndef GHOST_H
#define GHOST_H

#include <SFML/Graphics.hpp>
#include <array>
#include <random>
#include <string>

// Direction enum, shared by Pac-Man and the ghosts.
enum class Direction { NONE, UP, DOWN, LEFT, RIGHT };

sf::Vector2i dirDelta(Direction d);
int dirIndex(Direction d);   // up=0, down=1, left=2, right=3 (texture row order)

class Map;

// Ghost — a single ghost entity + the simple greedy AI ported from ref.cpp:
// at each tile centre it picks the non-reversing option that minimises (or, when
// frightened, maximises) the straight-line distance to Pac-Man, with a 15%
// random deviation. Eaten ghosts turn into eyes and head back to the house.
class Ghost {
public:
	Ghost() = default;

	void init(int index, const std::string& name);   // index 0..3, sprite folder name
	bool loadTextures();
	void reset(sf::Vector2f spawnTile, sf::Vector2f homeTile);

	// Advance one frame. baseDist is TILE-units-per-frame for a full-speed
	// entity; the ghost scales it by its own state (dead/frightened/normal).
	void update(float dt, float baseDist, sf::Vector2f pacPos, const Map& map, std::mt19937& rng);

	void setFrightened(float seconds);
	void eat();                                       // becomes eyes, heads home

	bool isFrightened() const { return frightened_ > 0.f; }
	bool isDead() const { return dead_; }
	sf::Vector2f pos() const { return pos_; }

	void render(sf::RenderWindow& window);

private:
	bool centered(sf::Vector2f p) const;
	void step(float dist);
	void moveGhost(float dist, sf::Vector2f pac, const Map& map, std::mt19937& rng);

	int index_ = 0;
	std::string name_;
	sf::Vector2f pos_{};
	Direction dir_ = Direction::UP;
	float animTimer_ = 0.f;
	int animFrame_ = 0;
	float frightened_ = 0.f;   // seconds of fright remaining
	bool dead_ = false;        // eyes mode (eaten, returning home)
	sf::Vector2f home_{ 9.f, 9.f };

	std::array<std::array<sf::Texture, 2>, 4> normal_;        // [dir][frame]
	std::array<std::array<sf::Texture, 2>, 4> frightenedTex_; // [dir][frame]
	std::array<std::array<sf::Texture, 2>, 4> deadTex_;       // [dir][frame]
};

#endif
