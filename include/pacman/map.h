#ifndef MAP_H
#define MAP_H

#include <SFML/Graphics.hpp>
#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// Map — owns the maze grid and draws it. Ported from ref.cpp's Game map logic.
// Positions are expressed in TILE units elsewhere in the game; this class only
// deals with integer tile coordinates and pixel rendering (px = tile*TILE, with
// a HUD strip offset at the top).
class Map {
public:
	static constexpr int   COLS = 19;
	static constexpr int   ROWS = 21;
	static constexpr float TILE = 22.f;   // sprite tiles are 22x22
	static constexpr float HUD = 40.f;    // HUD strip height at top

	enum class Eat { None, Dot, Power };

	Map() = default;

	// Load wall / edible textures. Returns false if a required texture is missing.
	bool loadTextures();

	// Generate a fresh maze template (mapData_) for the given seed.
	void loadGenerated(uint32_t seed);

	// Restore the live grid from the template, recount pellets, rescan spawns.
	void reset();

	// ---- live-grid queries --------------------------------------------------
	char tileChar(int c, int r) const;
	bool isWall(int c, int r) const;          // blocks pac-man (walls + ghost door)
	bool isWallForGhost(int c, int r) const;  // ghosts may pass the door
	bool isGhostHouse(int c, int r) const;    // ghost-house interior tiles

	// Consume a pellet at (c, r) if present; updates the pellet count.
	Eat consume(int c, int r);
	int pelletsRemaining() const { return pellets_; }

	// ---- spawn info (tile units) -------------------------------------------
	sf::Vector2f pacSpawn() const { return pacSpawn_; }
	const std::array<sf::Vector2f, 4>& ghostSpawns() const { return ghostSpawns_; }
	sf::Vector2f ghostHome() const { return ghostHome_; }

	void render(sf::RenderWindow& window);

	// tile -> pixel (including HUD offset), shared by all game entities.
	static sf::Vector2f toPixel(sf::Vector2f tile) {
		return { tile.x * TILE, tile.y * TILE + HUD };
	}

private:
	std::vector<std::string> mapData_;   // original template
	std::vector<std::string> grid_;      // live state (pellets consumed etc.)
	int pellets_ = 0;

	sf::Vector2f pacSpawn_{ 9.f, 16.f };
	std::array<sf::Vector2f, 4> ghostSpawns_{};   // blinky, inky, pinky, clyde
	sf::Vector2f ghostHome_{ 9.f, 9.f };

	std::unordered_map<std::string, sf::Texture> textures_;
	std::unordered_map<char, std::string> wallMap_;

	void drawSprite(sf::RenderWindow& window, const std::string& key, sf::Vector2f pixel);
};

#endif
