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

		void set(int col, int row, char c);
		void hLine(int col, int row, int len, char c);
		void vLine(int col, int row, int len, char c);
};


#endif