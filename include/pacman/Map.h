#ifndef MAP_H
#define MAP_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>

enum class TileType {
	EMPTY,
	WALL,
	DOT,
	POWER_PELLET,
	FRUIT,
	GHOST_SPAWN,
	PLAYER_SPAWN
};

class Map {
public:
	Map() = default;
	~Map() = default;

	bool load(const std::string& mapPath);
	void render(sf::RenderWindow& window);

	TileType getTileAt(int x, int y) const;
	void removeDot(int x, int y);

	int getWidth() const { return width_; }
	int getHeight() const { return height_; }
	int getTileSize() const { return tileSize_; }
	sf::Vector2f getPlayerSpawnPos() const { return playerSpawnPos_; }
	int getTotalDots() const { return totalDots_; }

private:
	// Textures - stored first
	sf::Texture dotTexture_;
	sf::Texture powerPelletTexture_;
	std::unordered_map<char, sf::Texture> wallTextures_;

	// Tile data
	std::vector<std::vector<TileType>> tileTypes_;
	std::vector<std::vector<bool>> hasDot_;
	std::vector<std::vector<bool>> hasPowerPellet_;

	// Sprites - created after textures are loaded. Grid-aligned: sprites_[y][x]
	// corresponds exactly to tile (x, y); empty optional means "no sprite here".
	std::vector<std::vector<std::optional<sf::Sprite>>> sprites_;

	int width_ = 0;
	int height_ = 0;
	int tileSize_ = 16;
	int totalDots_ = 0;
	sf::Vector2f playerSpawnPos_;

	bool loadTextures();
	TileType charToTileType(char c) const;
};

#endif