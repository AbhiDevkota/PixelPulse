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

class Ghost {
public:
	Ghost() = default;

	void init(int index, const std::string& name);
	bool loadTextures();
	void reset(sf::Vector2f spawnTile, sf::Vector2f homeTile);

	void update(float dt, float baseDist, sf::Vector2f pacPos, const Map& map,
	            const std::array<sf::Vector2f, 4>& allGhostPos, std::mt19937& rng);

	void setFrightened(float seconds);
	void eat();

	bool isFrightened() const { return frightened_ > 0.f; }
	bool isDead() const { return dead_; }
	bool isReviving() const { return reviving_; }
	sf::Vector2f pos() const { return pos_; }

	void render(sf::RenderWindow& window);

private:
	bool centered(sf::Vector2f p) const;
	void step(float dist);
	void moveGhost(float dist, sf::Vector2f pac, const Map& map,
	               const std::array<sf::Vector2f, 4>& allGhostPos, std::mt19937& rng);
	Direction bfsToTarget(const Map& map, sf::Vector2f from, sf::Vector2f to) const;

	int index_ = 0;
	std::string name_;
	sf::Vector2f pos_{};
	sf::Vector2f spawnTile_{};
	Direction dir_ = Direction::UP;
	bool wasCentered_ = false;
	float animTimer_ = 0.f;
	int animFrame_ = 0;
	float frightened_ = 0.f;
	bool dead_ = false;
	bool reviving_ = false;
	float reviveTimer_ = 0.f;
	sf::Vector2f home_{ 9.f, 9.f };

	std::array<std::array<sf::Texture, 2>, 4> normal_;
	std::array<std::array<sf::Texture, 2>, 4> frightenedTex_;
	std::array<std::array<sf::Texture, 2>, 4> deadTex_;
};

#endif
