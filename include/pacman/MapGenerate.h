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

	// naya: yo function le missing dot-field / room clusters haru paint garxa
	// (rows 1-8, 20-23, 26-29) - pahile yo part khaali (' ') nai rahanthyo,
	// tyahi le garda screenshot ma disconnected/floating wall dekhinthyo.
	void paintDotField();

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