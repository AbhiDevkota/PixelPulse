#include "pacman/MapGenerate.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <random>
#include <deque>
#include <utility>
#include <algorithm>

//using namespace std;

// ===================== SEEDED RNG (xorshift32, matches the TS prototype) =====================
namespace {

	struct Xorshift32 {
		uint32_t s;
		explicit Xorshift32(uint32_t seed) : s(seed ? seed : 1u) {}

		uint32_t next() {
			uint32_t x = s;
			x ^= x << 13;
			x ^= x >> 17;
			x ^= x << 5;
			s = x;
			return x;
		}

		double randf() { return static_cast<double>(next()) / 4294967296.0; }
		int randi(int maxExclusive) { return static_cast<int>(randf() * maxExclusive); }
	};

	constexpr int DCOL[4] = { 0, 1, 0, -1 };
	constexpr int DROW[4] = { -1, 0, 1, 0 };

	// ---- corridor carving on a local half-width field buffer ----
	// open[y][x] == true means walkable. Only the left half (x in [0, halfW))
	// is carved; the caller mirrors it into the right half of the real grid.

	void carveSkeleton(std::vector<std::vector<bool>>& open, int halfW, int h, Xorshift32& rng) {
		std::vector<int> hRows = { 0, h - 1 };
		if (h > 4) hRows.push_back(h / 2);
		if (h > 7) hRows.push_back(h / 3);

		for (int y : hRows) {
			for (int x = 0; x < halfW; ++x) open[y][x] = true;
		}

		std::vector<int> vCols = { 0 };
		vCols.push_back(1 + rng.randi(2));          // inner column: 1 or 2
		if (halfW > 6) vCols.push_back(halfW - 1);  // column hugging the centre divider

		for (int x : vCols) {
			for (int y = 0; y < h; ++y) open[y][x] = true;
		}

		// A few extra random connector cells so every field looks different.
		for (int y = 1; y < h - 1; ++y) {
			for (int x = 1; x < halfW - 1; ++x) {
				if (open[y][x]) continue;
				if (rng.randf() < 0.06) open[y][x] = true;
			}
		}
	}

	void breakIntersections(std::vector<std::vector<bool>>& open, int halfW, int h, Xorshift32& rng) {
		for (int y = 1; y < h - 1; ++y) {
			for (int x = 1; x < halfW - 1; ++x) {
				if (!open[y][x]) continue;
				int n = 0;
				for (int d = 0; d < 4; ++d) {
					int nx = x + DCOL[d], ny = y + DROW[d];
					if (nx < 0 || nx >= halfW || ny < 0 || ny >= h) continue;
					if (open[ny][nx]) n++;
				}
				// Only thin out genuine crossroads (3+ open neighbours) so we
				// don't accidentally sever a simple corridor.
				if (n >= 3 && rng.randf() < 0.35) open[y][x] = false;
			}
		}
	}

	// A dead end is an open cell with exactly one open neighbour (treating the
	// field boundary itself as "open", since it connects to the border
	// corridor). Punch a random adjacent wall to give it a second way out.
	void removeDeadEnds(std::vector<std::vector<bool>>& open, int halfW, int h, Xorshift32& rng) {
		for (int pass = 0; pass < 4; ++pass) {
			bool changed = false;
			for (int y = 1; y < h - 1; ++y) {
				for (int x = 1; x < halfW - 1; ++x) {
					if (!open[y][x]) continue;
					int openCount = 0;
					std::vector<int> wallDirs;
					for (int d = 0; d < 4; ++d) {
						int nx = x + DCOL[d], ny = y + DROW[d];
						if (nx < 0 || nx >= halfW || ny < 0 || ny >= h) { openCount++; continue; }
						if (open[ny][nx]) openCount++;
						else wallDirs.push_back(d);
					}
					if (openCount == 1 && !wallDirs.empty()) {
						int d = wallDirs[rng.randi(static_cast<int>(wallDirs.size()))];
						int nx = x + DCOL[d], ny = y + DROW[d];
						if (nx >= 0 && nx < halfW && ny >= 0 && ny < h) {
							open[ny][nx] = true;
							changed = true;
						}
					}
				}
			}
			if (!changed) break;
		}
	}

	bool wouldCreateDeadEnd(const std::vector<std::vector<bool>>& open, int x, int y, int halfW, int h) {
		for (int d = 0; d < 4; ++d) {
			int nx = x + DCOL[d], ny = y + DROW[d];
			if (nx <= 0 || nx >= halfW - 1 || ny <= 0 || ny >= h - 1) continue;
			if (!open[ny][nx]) continue;
			int openCount = 0;
			for (int dd = 0; dd < 4; ++dd) {
				int nnx = nx + DCOL[dd], nny = ny + DROW[dd];
				if (nnx == x && nny == y) continue; // would become the new wall
				if (nnx < 0 || nnx >= halfW || nny < 0 || nny >= h) { openCount++; continue; }
				if (open[nny][nnx]) openCount++;
			}
			if (openCount <= 1) return true;
		}
		return false;
	}

	// Break up leftover 3x3 open blocks so rooms don't feel like empty halls.
	// Never blocks the outer field edge (that's the border corridor).
	void breakRooms(std::vector<std::vector<bool>>& open, int halfW, int h, Xorshift32& rng) {
		for (int pass = 0; pass < 3; ++pass) {
			bool found = false;
			for (int y = 1; y < h - 3; ++y) {
				for (int x = 1; x < halfW - 3; ++x) {
					bool allOpen = true;
					for (int dy = 0; dy < 3 && allOpen; ++dy)
						for (int dx = 0; dx < 3 && allOpen; ++dx)
							if (!open[y + dy][x + dx]) allOpen = false;
					if (!allOpen) continue;
					found = true;

					int cx = x + 1, cy = y + 1;
					if (!wouldCreateDeadEnd(open, cx, cy, halfW, h)) {
						open[cy][cx] = false;
						continue;
					}

					std::vector<std::pair<int, int>> candidates;
					for (int dy = 0; dy < 3; ++dy) {
						for (int dx = 0; dx < 3; ++dx) {
							if (dx == 1 && dy == 1) continue;
							int px = x + dx, py = y + dy;
							if (px <= 0 || px >= halfW - 1 || py <= 0 || py >= h - 1) continue;
							candidates.emplace_back(px, py);
						}
					}
					for (int i = static_cast<int>(candidates.size()) - 1; i > 0; --i) {
						int j = rng.randi(i + 1);
						std::swap(candidates[i], candidates[j]);
					}
					for (auto& pos : candidates) {
						if (!wouldCreateDeadEnd(open, pos.first, pos.second, halfW, h)) {
							open[pos.second][pos.first] = false;
							break;
						}
					}
				}
			}
			if (!found) break;
		}
	}

} // namespace

// ===================== GENERATE =====================
void MapGenerator::generate(uint32_t seed) {
	if (seed == 0) {
		std::random_device rd;
		seed = rd();
	}

	initGrid();
	paintOuterBorder();
	generateDotField(seed);   // dynamic replacement for the old static paintDotField()
	pasteGhostHouse();
	fixConnectivity();        // guarantee every open tile is reachable from spawn
	autotileWalls();          // give generated walls proper corner/straight glyphs
	placePowerPellets(seed);

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

// Seeded procedural replacement for the old hand-typed paintDotField().
// Carves rows 1-8 (top field) and rows 20-29 (bottom field), cols 1-26,
// mirrored left/right around the centre divider (cols 13/14). Border
// cells (col 0/27) and the ghost house (rows 9-19) are never touched here.
void MapGenerator::generateDotField(uint32_t seed) {
	struct FieldSpec { int rowStart, rowEnd; uint32_t seedOffset; };
	const FieldSpec fields[2] = {
		{ 1,  8,  0x1u },
		{ 20, 29, 0x9E3779B9u },
	};

	constexpr int fullW = 26;  // cols 1..26
	constexpr int halfW = 13;  // cols 1..13, mirrored to 14..26
	constexpr int colOffset = 1;

	for (const auto& f : fields) {
		int h = f.rowEnd - f.rowStart + 1;
		uint32_t s = seed ^ f.seedOffset;
		Xorshift32 rng(s == 0 ? 1u : s);

		std::vector<std::vector<bool>> open(h, std::vector<bool>(halfW, false));
		carveSkeleton(open, halfW, h, rng);
		breakIntersections(open, halfW, h, rng);
		removeDeadEnds(open, halfW, h, rng);
		breakRooms(open, halfW, h, rng);
		removeDeadEnds(open, halfW, h, rng);

		for (int y = 0; y < h; ++y) {
			int row = f.rowStart + y;
			for (int x = 0; x < halfW; ++x) {
				char c = open[y][x] ? '*' : '2'; // '2' = temp wall placeholder, shaped later by autotileWalls()
				set(colOffset + x, row, c);
				set(colOffset + (fullW - 1 - x), row, c); // mirrored column
			}
		}
	}
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

bool MapGenerator::isWalkable(char c) const {
	return !(c == '!' || c == '@' || c == '#' || c == '$' || c == '%' || c == '^'
		|| (c >= '1' && c <= '6'));
}

bool MapGenerator::isProtectedWall(int col, int row) const {
	if (col <= 0 || col >= COLS - 1 || row <= 0 || row >= ROWS - 1) return true; // outer border ring
	if (col >= 6 && col <= 21 && row >= 9 && row <= 19) return true;             // hand-authored ghost house block
	return false;
}

// Whole-map BFS from the player spawn tile ('f'). Any unreachable dot or
// power pellet gets a shortest safe path carved to it, opening only our
// own generated '2' wall placeholders -- never the border or the ghost
// house art. Plain empty floor (e.g. the unused tunnel side-alcoves that
// exist in the original hand-drawn border art) is intentionally NOT
// required to be reachable -- it never was in the original static map
// either, since those pockets have no dots and sit outside normal play.
void MapGenerator::fixConnectivity() {
	int startCol = -1, startRow = -1;
	for (int row = 0; row < ROWS; ++row) {
		for (int col = 0; col < COLS; ++col) {
			if (grid_[row][col] == 'f') { startCol = col; startRow = row; } // last 'f' wins, matches Map::load's scan order
		}
	}
	if (startCol < 0) return;

	auto needsReach = [](char c) { return c == '*' || c == 'o'; };

	for (int iteration = 0; iteration < 40; ++iteration) {
		std::vector<std::vector<bool>> visited(ROWS, std::vector<bool>(COLS, false));
		std::vector<std::pair<int, int>> stack{ { startCol, startRow } };
		while (!stack.empty()) {
			auto cell = stack.back(); stack.pop_back();
			int col = cell.first, row = cell.second;
			if (col < 0 || col >= COLS || row < 0 || row >= ROWS) continue;
			if (visited[row][col]) continue;
			if (!isWalkable(grid_[row][col])) continue;
			visited[row][col] = true;
			for (int d = 0; d < 4; ++d) stack.push_back({ col + DCOL[d], row + DROW[d] });
		}

		int uc = -1, ur = -1;
		for (int row = 1; row < ROWS - 1 && uc < 0; ++row) {
			for (int col = 1; col < COLS - 1; ++col) {
				if (needsReach(grid_[row][col]) && !visited[row][col]) { uc = col; ur = row; break; }
			}
		}
		if (uc < 0) return; // every dot/pellet is reachable

		std::vector<std::vector<bool>> seen(ROWS, std::vector<bool>(COLS, false));
		std::vector<std::vector<std::pair<int, int>>> parent(ROWS, std::vector<std::pair<int, int>>(COLS, { -1, -1 }));
		std::deque<std::pair<int, int>> queue{ { uc, ur } };
		seen[ur][uc] = true;
		bool connected = false;

		while (!queue.empty()) {
			auto cell = queue.front(); queue.pop_front();
			int col = cell.first, row = cell.second;
			if (visited[row][col]) {
				int c = col, r = row;
				bool openedAny = false;
				while (!(c == uc && r == ur)) {
					if (grid_[r][c] == '2') { grid_[r][c] = '*'; openedAny = true; }
					auto p = parent[r][c];
					c = p.first; r = p.second;
				}
				// If the route was already all-open (nothing left to convert),
				// then (uc, ur) was truly unreachable through modifiable walls
				// only -- bail instead of burning iterations on a no-op.
				connected = openedAny;
				break;
			}
			for (int d = 0; d < 4; ++d) {
				int nc = col + DCOL[d], nr = row + DROW[d];
				if (nc < 0 || nc >= COLS || nr < 0 || nr >= ROWS) continue;
				if (seen[nr][nc]) continue;
				if (isProtectedWall(nc, nr)) continue; // never route through border / ghost house
				seen[nr][nc] = true;
				parent[nr][nc] = { col, row };
				queue.push_back({ nc, nr });
			}
		}
		if (!connected) return; // no safe route found this pass; bail rather than loop forever
	}
}

void MapGenerator::autotileWalls() {
	for (int row = 0; row < ROWS; ++row) {
		for (int col = 0; col < COLS; ++col) {
			if (grid_[row][col] != '2') continue; // only re-shape our own placeholders

			bool openUp = row > 0 && isWalkable(grid_[row - 1][col]);
			bool openDown = row < ROWS - 1 && isWalkable(grid_[row + 1][col]);
			bool openLeft = col > 0 && isWalkable(grid_[row][col - 1]);
			bool openRight = col < COLS - 1 && isWalkable(grid_[row][col + 1]);

			char shape;
			if (openDown && openRight)      shape = '1'; // left-top
			else if (openDown && openLeft)  shape = '3'; // right-top
			else if (openUp && openRight)   shape = '5'; // left-bottom
			else if (openUp && openLeft)    shape = '6'; // right-bottom
			else if (openLeft || openRight) shape = '2'; // horizontal
			else                             shape = '4'; // vertical (also default fallback)

			grid_[row][col] = shape;
		}
	}
}

void MapGenerator::placePowerPellets(uint32_t seed) {
	uint32_t s = seed ^ 0xBEEFu;
	Xorshift32 rng(s == 0 ? 1u : s);

	struct Corner { int startCol, startRow, dCol, dRow; };
	const Corner corners[4] = {
		{ 1,        1,        1,  1 },
		{ COLS - 2, 1,       -1,  1 },
		{ 1,        ROWS - 2, 1, -1 },
		{ COLS - 2, ROWS - 2,-1, -1 },
	};

	for (const auto& c : corners) {
		bool placed = false;
		for (int d = 0; d < 10 && !placed; ++d) {
			std::vector<std::pair<int, int>> candidates;
			for (int dy = 0; dy <= d; ++dy) {
				for (int dx = 0; dx <= d; ++dx) {
					int col = c.startCol + dx * c.dCol;
					int row = c.startRow + dy * c.dRow;
					if (col <= 0 || col >= COLS - 1 || row <= 0 || row >= ROWS - 1) continue;
					if (grid_[row][col] == '*') candidates.emplace_back(col, row);
				}
			}
			if (!candidates.empty()) {
				auto pick = candidates[rng.randi(static_cast<int>(candidates.size()))];
				grid_[pick.second][pick.first] = 'o';
				placed = true;
			}
		}
	}
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