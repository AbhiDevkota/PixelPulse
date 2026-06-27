#include "pacman/MapGenerate.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

//using namespace std;

void MapGenerator::generate(uint32_t) {	//seed generation
	initGrid();
	paintOuterBorder();
	pasteGhostHouse();
}

bool MapGenerator::save(const std::string& path)const{
	std::ofstream file(path);
	if(!file.is_open()){
		std::cerr << "Map gen cant open for writing: " << path << std::endl;
		return false;
	}
	for (int row =0; row <ROWS; ++row){
		for (int col = 0; col < COLS; ++col) {
			file << grid_[row][col];
		}
		file << "\r\n";
	}
	return file.good();
}