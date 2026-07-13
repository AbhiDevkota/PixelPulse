#ifndef MAPGENERATE_H
#define MAPGENERATE_H

#include <vector>
#include <string>
#include <cstdint>

// Procedural Pac-Man maze generator, ported from the reference single-file
// build (ref.cpp). Produces a 19x21 renderable char grid using a seeded
// XorShift32 PRNG so the same seed always reproduces the same maze.
//
// Render char scheme (consumed by Map):
//   walls  : ! @ # $ % ^   (double-line box glyphs)
//   dot    : *   power pellet : 0   ghost door : -   empty : space
//   spawns : o (pac-man)  b i p c (blinky / inky / pinky / clyde)
class MapGenerator {
public:
	static constexpr int COLS = 19;
	static constexpr int ROWS = 21;

	MapGenerator() = default;

	// Fills the grid with a freshly generated maze for the given seed.
	void generate(uint32_t seed = 0);

	// Char at (col, row) in the generated grid; ' ' if out of bounds.
	char at(int col, int row) const;

private:
	std::vector<std::string> grid_;
};

#endif
