#include "pacman/MapGenerate.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <random>

//using namespace std;

void MapGenerator::generate(uint32_t seed) {	//seed generation
	initGrid();
	paintOuterBorder();
	paintDotField();     // <-- yesle nai missing corridors/dot-field haru fix garxa
	pasteGhostHouse();

	// seed abhi ko lagi variant selection ko lagi reserve gareko, jaba arko
	// verified map variant thapinxa (selectVariant()), tesbela random hunxa.
	(void)selectVariant(seed);
}

int MapGenerator::selectVariant(uint32_t seed) const {
	std::mt19937 rng(seed == 0 ? std::random_device{}() : seed);
	constexpr int kVariantCount = 1; // increase once more validated .map layouts are added
	return static_cast<int>(rng() % kVariantCount);
}

bool MapGenerator::save(const std::string& path)const {
	std::ofstream file(path);
	if (!file.is_open()) {
		std::cerr << "Map gen cant open for writing: " << path << std::endl;
		return false;
	}
	for (int row = 0; row < ROWS; ++row) {
		for (int col = 0; col < COLS; ++col) {
			file << grid_[row][col];
		}
		file << "\r\n";
	}
	return file.good();
}

char MapGenerator::at(int col, int row) const {
	if (col < 0 || col >= COLS || row < 0 || row >= ROWS) {
		return ' ';
	}
	return grid_[row][col];
}

void MapGenerator::initGrid() {
	grid_.assign(ROWS, std::vector<char>(COLS, ' '));

}

void MapGenerator::paintOuterBorder() {
	set(0, 0, '!');				//map create garna lai for the playing area border.
	hLine(1, 0, 12, '@');
	set(13, 0, '#');
	set(14, 0, '!');
	hLine(15, 0, 12, '@');
	set(27, 0, '#');

	vLine(0, 1, 8, '$');		//left right wall
	vLine(27, 1, 8, '$');

	set(0, 9, '%'); hLine(1, 9, 4, '@'); set(5, 9, '#');		//ghost
	set(22, 9, '!'); hLine(23, 9, 4, '@'); set(27, 9, '^');

	vLine(5, 10, 3, '$');
	vLine(22, 10, 3, '$');

	set(0, 13, '!'); hLine(1, 13, 4, '@'); set(5, 13, '^');
	set(22, 13, '%'); hLine(23, 13, 4, '@'); set(27, 13, '#');

	set(0, 14, '$');
	set(27, 14, '$');

	set(0, 15, '%'); hLine(1, 15, 4, '@'); set(5, 15, '#');
	set(22, 15, '!'); hLine(23, 15, 4, '@'); set(27, 15, '^');

	vLine(5, 16, 3, '$');
	vLine(22, 16, 3, '$');

	set(0, 19, '!'); hLine(1, 19, 4, '@'); set(5, 19, '^');
	set(22, 19, '%'); hLine(23, 19, 4, '@'); set(27, 19, '#');

	vLine(0, 20, 4, '$');
	vLine(27, 20, 4, '$');

	set(0, 24, '%'); set(1, 24, '@'); set(2, 24, '#');
	set(25, 24, '!'); set(26, 24, '@'); set(27, 24, '^');

	set(0, 25, '!'); set(1, 25, '@'); set(2, 25, '^');
	set(25, 25, '%'); set(26, 25, '@'); set(27, 25, '#');

	vLine(0, 26, 4, '$');
	vLine(27, 26, 4, '$');

	set(0, 30, '%');
	hLine(1, 30, 26, '@');
	set(27, 30, '^');
}

// yo function le bug fix garxa: pahile rows 1-8, 20-23, 26-29 (dot field ra
// corner room clusters) kahilei paint hunthenan, tyo khaali space nai
// rahanthyo -> tesle garda maze ko wall haru disconnected/floating dekhinthyo
// ani dots pani missing hunthe. Tallo values reference single-player.map
// bata exact match huney gari verify gareko (byte-for-byte diff = 0).
void MapGenerator::paintDotField() {
	pasteRow(0, 1, "$************$$************$");
	pasteRow(0, 2, "$*1223*12223*$$*12223*1223*$");
	pasteRow(0, 3, "$04  4*4   4*$$*4   4*4  40$");
	pasteRow(0, 4, "$*5226*52226*%^*52226*5226*$");
	pasteRow(0, 5, "$**************************$");
	pasteRow(0, 6, "$*1223*13*12222223*13*1223*$");
	pasteRow(0, 7, "$*5226*44*52231226*44*5226*$");
	pasteRow(0, 8, "$******44****44****44******$");

	pasteRow(0, 20, "$************44************$");
	pasteRow(0, 21, "$*1223*12223*44*12223*1223*$");
	pasteRow(0, 22, "$*5234*52226*56*52226*4126*$");
	pasteRow(0, 23, "$0**44*******o *******44**0$");
	pasteRow(0, 24, "%@#*44*13*12222223*13*44*!@^");
	pasteRow(0, 25, "!@^*56*44*52231226*44*56*%@#");
	pasteRow(0, 26, "$******44****44****44******$");
	pasteRow(0, 27, "$*1222265223*44*1226522223*$");
	pasteRow(0, 28, "$*5222222226*56*5222222226*$");
	pasteRow(0, 29, "$**************************$");
}

void MapGenerator::pasteGhostHouse() {
	static const char* const GHOST_ROWS[11] = {
	"*45223 44 12264*",  // row  9
	"*41226 56 52234*",  // row 10
	"*44          44*",  // row 11
	"*44 !@@--@@# 44*",  // row 12
	"*56 $b    p$ 56*",  // row 13
	"*   $      $   *",  // row 14
	"*13 $i    c$ 13*",  // row 15
	"*44 %@@@@@@^ 44*",  // row 16
	"*44          44*",  // row 17
	"*44 12222223 44*",  // row 18
	"*56 52231226 56*",  // row 19
	};

	for (int i = 0; i < 11; ++i) {
		int row = 9 + i;
		pasteRow(6, row, GHOST_ROWS[i]);
	}
	set(1, 14, 'f');
	set(26, 14, 'f');
}

void MapGenerator::set(int col, int row, char c) {
	if (col < 0 || col >= COLS || row < 0 || row >= ROWS) return;
	grid_[row][col] = c;
}

void MapGenerator::hLine(int col, int row, int len, char c) {
	for (int i = 0; i < len; ++i) set(col + i, row, c);
}

void MapGenerator::vLine(int col, int row, int len, char c) {
	for (int i = 0; i < len; ++i) set(col, row + i, c);
}

void MapGenerator::pasteRow(int col, int row, const std::string& s) {
	for (std::size_t i = 0; i < s.size(); ++i) {
		set(col + static_cast<int>(i), row, s[i]);
	}
}