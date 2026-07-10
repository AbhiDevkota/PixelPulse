#ifndef MAPGENERATE_H
#define MAPGENERATE_H

#include <vector>
#include <string>
#include <cstdint>

class MapGenerator {
public:
	static constexpr int COLS = 28;
	static constexpr int ROWS = 31;

	MapGenerator() = default;

	void generate(uint32_t seed = 0);

	bool save(const std::string& path) const;

	char at(int col, int row) const; //const add garyo bhane function ko last ma tyo read type function matra hunxa

private:
	using Grid = std::vector<std::vector<char>>;
	Grid grid_;

	void initGrid();

	void paintOuterBorder();

	void pasteGhostHouse();

	// Seeded procedural corridor generator. Replaces the old static
	// paintDotField(). Carves the two open "field" regions (rows 1-8 and
	// 20-29) using randomized corridor carving, dead-end removal and
	// room-breaking, mirrored left/right for symmetry. Leaves the border
	// (row 0/29/col 0/27) and the ghost-house block (cols 6-21, rows 9-19)
	// completely untouched.
	void generateDotField(uint32_t seed);

	// Whole-map BFS from the player spawn ('f'). If any dot/open tile is
	// unreachable, carves the shortest safe path to it, opening only our
	// own generated walls ('2' placeholders) and never the border or the
	// hand-authored ghost house.
	void fixConnectivity();

	// Converts leftover '2' wall placeholders left by generateDotField()
	// into properly oriented corner/straight wall glyphs based on their
	// open neighbours (same alphabet the ghost house / dot field already
	// use: 1=left-top, 2=horizontal, 3=right-top, 4=vertical,
	// 5=left-bottom, 6=right-bottom).
	void autotileWalls();

	// Drops one power pellet near each of the four map corners, searching
	// outward in expanding rings until a dot tile is found.
	void placePowerPellets(uint32_t seed);

	bool isWalkable(char c) const;
	bool isProtectedWall(int col, int row) const;

	// TODO: variant selection using `seed` once more validated map
	// variants are added (see selectVariant()). Right now there is
	// only 1 authentic layout, so seed doesn't change the output yet,
	// but the plumbing is there for when more variants exist.
	int selectVariant(uint32_t seed) const;

	void set(int col, int row, char c);
	void hLine(int col, int row, int len, char c);
	void vLine(int col, int row, int len, char c);
	void pasteRow(int col, int row, const std::string& s); // generic version of pasteGhostHouse's per-row paste
};


#endif