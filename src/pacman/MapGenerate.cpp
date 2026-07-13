#include "pacman/MapGenerate.h"

#include <algorithm>
#include <array>
#include <random>
#include <string>
#include <utility>
#include <vector>

// ---------------------------------------------------------------------------
// Maze generation, ported verbatim (behaviourally) from ref.cpp's namespace
// helpers + MapGenerator class. Kept private to this translation unit; the
// public MapGenerator (declared in the header) is a thin wrapper over it.
// ---------------------------------------------------------------------------
namespace {

	constexpr int kCols = 19;
	constexpr int kRows = 21;

	// XorShift32 — deterministic PRNG. All maze randomness flows through this,
	// seeded once per round, so the same seed always reproduces the same maze.
	struct XorShift32 {
		uint32_t state;
		explicit XorShift32(uint32_t seed) : state(seed != 0 ? seed : 0x9E3779B9u) {}

		uint32_t next() {
			uint32_t x = state;
			x ^= x << 13;
			x ^= x >> 17;
			x ^= x << 5;
			state = x;
			return x;
		}
		bool coinFlip() { return (next() & 1u) != 0u; }
		bool chance(int percent) { return int(next() % 100u) < percent; }          // percent, 0-100
		bool chancePerMille(int perMille) { return int(next() % 1000u) < perMille; } // per-mille, 0-1000
		uint32_t below(uint32_t n) { return n == 0 ? 0 : next() % n; }
	};

	// MapGenerator — implements the 20-phase maze generation rulebook on a
	// 21 row x 19 col grid (rows/cols 0-indexed; row0/row20 and col0/col18 are
	// the outer border).
	class MazeGen {
	public:
		static std::vector<std::string> generate(uint32_t seed) {
			XorShift32 rng(seed);
			std::vector<std::string> g(kRows, std::string(kCols, 'W'));

			const int vColSeed = rng.coinFlip() ? 3 : 4;                 // Phase 1
			const std::array<int, 7> hRows = { 1, 4, 7, 12, 14, 16, 19 };
			const std::array<int, 3> vCols = { 1, vColSeed, 5 };

			phase2_horizontalCorridors(g, hRows);
			phase3_verticalCorridors(g, vCols);
			phase4_centerColumn(g, rng, hRows);
			phase5_breakIntersections(g, rng, hRows, vCols);
			phase6_breakHorizontalRuns(g, rng, hRows, vCols);
			phase7_verticalSidePaths(g, rng, hRows, vCols);
			phase8_mirror(g);
			phase9_centerConnections(g);
			phase10_ghostHouse(g);
			phase11_ghostHouseAccess(g);
			phase12_tunnel(g);
			phase13_pacmanStart(g);
			phase14_removeDeadEnds(g, rng);          // Phase 14
			phase15_breakRooms(g, rng);               // Phase 15
			phase14_removeDeadEnds(g, rng);           // Phase 16 (re-run of 14)
			phase17_connectivityRepair(g);            // Phase 17
			phase15_breakRooms(g, rng);               // Phase 17.5 cleanup
			phase14_removeDeadEnds(g, rng);
			phase18_powerPellets(g, rng);
			phase19_finalReinforce(g);
			phase20_strayEmptyToDot(g);
			removeFloatingWallFragments(g, /*minComponentSize=*/3);
			phase15_breakRooms(g, rng);               // safety re-pass: fragment removal
			phase14_removeDeadEnds(g, rng);            // can only open tiles, never break anything,
			// but could in principle expose a new 3x3 room

			phase21_addLoopConnections(g, rng);        // Phase 21 — extra loops for route variety
			phase19_finalReinforce(g);                 // re-assert protected structures before locking width
			phase22_enforceSingleWidth(g, rng);        // Phase 22 — collapse every 2x2 to single-tile width

			return toRenderChars(g);
		}

	private:
		static constexpr int kPacX = 9, kPacY = 16;

		// ---- small helpers ------------------------------------------------------
		static bool inBounds(int x, int y) { return x >= 0 && x < kCols && y >= 0 && y < kRows; }
		static bool isGhostZone(int x, int y) { return x >= 6 && x <= 12 && y >= 8 && y <= 11; }

		static char get(const std::vector<std::string>& g, int x, int y) {
			if (!inBounds(x, y)) return 'W';
			return g[y][x];
		}
		static void set(std::vector<std::string>& g, int x, int y, char c) {
			if (inBounds(x, y)) g[y][x] = c;
		}
		static bool isOpenTile(char c) { return c == '.' || c == ' ' || c == 'P'; }
		static bool isPassable(char c) { return c != 'W'; }  // for Phase 17 flood-fill / BFS

		static void carveIfWall(std::vector<std::string>& g, int x, int y) {
			if (isGhostZone(x, y)) return;
			if (get(g, x, y) == 'W') set(g, x, y, '.');
		}

		// ---- Phase 2 — carve horizontal corridors --------------------------------
		static void phase2_horizontalCorridors(std::vector<std::string>& g, const std::array<int, 7>& hRows) {
			for (int y : hRows)
				for (int x = 1; x <= 9; ++x)
					carveIfWall(g, x, y);
		}

		// ---- Phase 3 — carve vertical corridors -----------------------------------
		static void phase3_verticalCorridors(std::vector<std::string>& g, const std::array<int, 3>& vCols) {
			for (int x : vCols)
				for (int y = 1; y <= 17; ++y)
					carveIfWall(g, x, y);
		}

		// ---- Phase 4 — open the center column --------------------------------------
		static void phase4_centerColumn(std::vector<std::string>& g, XorShift32& rng, const std::array<int, 7>& hRows) {
			for (int y : hRows) carveIfWall(g, 9, y);
			for (int y = 1; y <= 19; ++y) {
				if (std::find(hRows.begin(), hRows.end(), y) != hRows.end()) continue;
				if (isGhostZone(9, y)) continue;
				if (rng.chance(18)) set(g, 9, y, '.');
			}
		}

		// ---- Phase 5 — break up intersections with wall blocks ---------------------
		static void phase5_breakIntersections(std::vector<std::string>& g, XorShift32& rng,
			const std::array<int, 7>& hRows, const std::array<int, 3>& vCols) {
			const std::array<int, 4> cols = { vCols[0], vCols[1], vCols[2], 9 };
			for (int hRow : hRows) {
				if (hRow == 1 || hRow == 19) continue;
				for (int vCol : cols) {
					if (vCol == 1) continue;
					if (isGhostZone(vCol, hRow)) continue;
					int pct = (vCol == 9) ? 28 : 40;
					if (rng.chance(pct)) set(g, vCol, hRow, 'W');
				}
			}
		}

		// ---- Phase 6 — add wall blocks within horizontal corridors -----------------
		static void phase6_breakHorizontalRuns(std::vector<std::string>& g, XorShift32& rng,
			const std::array<int, 7>& hRows, const std::array<int, 3>& vCols) {
			const std::array<int, 4> cols = { vCols[0], vCols[1], vCols[2], 9 };
			for (int y : hRows) {
				if (y == 1 || y == 19 || y == 12) continue;
				for (int x = 1; x <= 9; ++x) {
					if (std::find(cols.begin(), cols.end(), x) != cols.end()) continue;
					if (get(g, x, y) != '.') continue;
					if (rng.chance(8)) set(g, x, y, 'W');
				}
			}
		}

		// ---- Phase 7 — vertical side paths between horizontal corridors ------------
		static void phase7_verticalSidePaths(std::vector<std::string>& g, XorShift32& rng,
			const std::array<int, 7>& hRows, const std::array<int, 3>& vCols) {
			for (int y = 1; y <= 19; ++y) {
				if (std::find(hRows.begin(), hRows.end(), y) != hRows.end()) continue;
				for (int x = 1; x <= 8; ++x) {
					if (isGhostZone(x, y)) continue;
					if (get(g, x, y) != 'W') continue;
					bool isVCol = std::find(vCols.begin(), vCols.end(), x) != vCols.end();
					if (isVCol) {
						if (rng.chance(22)) set(g, x, y, '.');
					}
					else {
						if (rng.chancePerMille(35)) set(g, x, y, '.');  // 3.5%
					}
				}
			}
		}

		// ---- Phase 8 — mirror left half to right half -------------------------------
		static void phase8_mirror(std::vector<std::string>& g) {
			for (int y = 1; y <= 19; ++y)
				for (int x = 1; x <= 8; ++x)
					if (g[y][x] == '.') set(g, 18 - x, y, '.');
		}

		// ---- Phase 9 — ensure center column has horizontal connections -------------
		static void phase9_centerConnections(std::vector<std::string>& g) {
			for (int y = 1; y <= 19; ++y) {
				if (get(g, 9, y) != '.') continue;
				if (get(g, 8, y) == 'W') set(g, 8, y, '.');
				if (get(g, 10, y) == 'W') set(g, 10, y, '.');
			}
		}

		// ---- Phase 10 — stamp down the ghost house -----------------------------------
		static void phase10_ghostHouse(std::vector<std::string>& g) {
			for (int x = 7; x <= 11; ++x)
				for (int y = 9; y <= 10; ++y)
					set(g, x, y, 'H');
			for (int x = 6; x <= 12; ++x) { set(g, x, 8, 'W'); set(g, x, 11, 'W'); }
			for (int y = 8; y <= 11; ++y) { set(g, 6, y, 'W'); set(g, 12, y, 'W'); }
			set(g, 9, 8, 'D');  // door
		}

		// ---- Phase 11 — ghost house access corridor -----------------------------------
		static void phase11_ghostHouseAccess(std::vector<std::string>& g) {
			set(g, 9, 7, '.');
			for (int x = 1; x <= 9; ++x)  if (get(g, x, 7) == 'W') set(g, x, 7, '.');
			for (int x = 9; x <= 17; ++x) if (get(g, x, 7) == 'W') set(g, x, 7, '.');
			for (int y = 1; y <= 7; ++y)  if (get(g, 9, y) == 'W') set(g, 9, y, '.');
		}

		// ---- Phase 12 — tunnel -----------------------------------------------------
		static void phase12_tunnel(std::vector<std::string>& g) {
			for (int x = 0; x < kCols; ++x) set(g, x, 12, ' ');
			set(g, 0, 12, ' ');
			set(g, kCols - 1, 12, ' ');
		}

		// ---- Phase 13 — pacman start area -------------------------------------------
		static void phase13_pacmanStart(std::vector<std::string>& g) {
			for (int x = 7; x <= 11; ++x)
				for (int y = 15; y <= 17; ++y)
					if (get(g, x, y) == 'W') set(g, x, y, '.');
			set(g, kPacX, kPacY, ' ');
			for (int y = 1; y <= 16; ++y) if (get(g, 9, y) == 'W') set(g, 9, y, '.');
			for (int x = 1; x <= 9; ++x)  if (get(g, x, 16) == 'W') set(g, x, 16, '.');
			for (int x = 9; x <= 17; ++x) if (get(g, x, 16) == 'W') set(g, x, 16, '.');
		}

		// ---- Phase 14 / 16 — remove dead ends -----------------------------------------
		static void phase14_removeDeadEnds(std::vector<std::string>& g, XorShift32& rng) {
			static const int dx[4] = { 0, 0, -1, 1 };
			static const int dy[4] = { -1, 1, 0, 0 };
			for (int pass = 0; pass < 4; ++pass) {
				bool anyFixed = false;
				for (int y = 0; y < kRows; ++y) {
					for (int x = 0; x < kCols; ++x) {
						if (!isOpenTile(g[y][x])) continue;
						int openCount = 0;
						for (int d = 0; d < 4; ++d)
							if (isOpenTile(get(g, x + dx[d], y + dy[d]))) ++openCount;
						if (openCount != 1) continue;

						std::vector<int> wallDirs;
						for (int d = 0; d < 4; ++d) {
							int nx = x + dx[d], ny = y + dy[d];
							if (get(g, nx, ny) == 'W' && !isGhostZone(nx, ny)) wallDirs.push_back(d);
						}
						if (!wallDirs.empty()) {
							int d = wallDirs[rng.below((uint32_t)wallDirs.size())];
							set(g, x + dx[d], y + dy[d], '.');
							anyFixed = true;
						}
					}
				}
				if (!anyFixed) break;
			}
		}

		// ---- Phase 15 / 17.5 — break up large 3x3 rooms --------------------------------
		static bool wouldCreateDeadEnd(const std::vector<std::string>& g, int x, int y) {
			static const int dx[4] = { 0, 0, -1, 1 };
			static const int dy[4] = { -1, 1, 0, 0 };
			for (int d = 0; d < 4; ++d) {
				int nx = x + dx[d], ny = y + dy[d];
				if (!isOpenTile(get(g, nx, ny))) continue;
				int openCount = 0;
				for (int d2 = 0; d2 < 4; ++d2) {
					int mx = nx + dx[d2], my = ny + dy[d2];
					if (mx == x && my == y) continue;  // this tile is about to become WALL
					if (isOpenTile(get(g, mx, my))) ++openCount;
				}
				if (openCount <= 1) return true;
			}
			return false;
		}

		static bool isProtectedFromWalling(int x, int y) {
			return x == 1 || x == 17 || y == 1 || y == 19 || isGhostZone(x, y);
		}

		static void phase15_breakRooms(std::vector<std::string>& g, XorShift32& rng) {
			for (int pass = 0; pass < 3; ++pass) {
				std::vector<std::pair<int, int>> rooms;
				for (int y = 0; y <= kRows - 3; ++y) {
					for (int x = 0; x <= kCols - 3; ++x) {
						bool allOpen = true;
						for (int yy = y; yy < y + 3 && allOpen; ++yy)
							for (int xx = x; xx < x + 3; ++xx)
								if (!isOpenTile(get(g, xx, yy))) { allOpen = false; break; }
						if (allOpen) rooms.emplace_back(x, y);
					}
				}
				if (rooms.empty()) break;

				for (auto& room : rooms) {
					int rx = room.first, ry = room.second;
					bool stillRoom = true;
					for (int yy = ry; yy < ry + 3 && stillRoom; ++yy)
						for (int xx = rx; xx < rx + 3; ++xx)
							if (!isOpenTile(get(g, xx, yy))) { stillRoom = false; break; }
					if (!stillRoom) continue;

					int cx = rx + 1, cy = ry + 1;
					if (!isProtectedFromWalling(cx, cy) && !wouldCreateDeadEnd(g, cx, cy)) {
						set(g, cx, cy, 'W');
						continue;
					}

					std::vector<std::pair<int, int>> edges;
					for (int yy = ry; yy < ry + 3; ++yy)
						for (int xx = rx; xx < rx + 3; ++xx)
							if (!(xx == cx && yy == cy) && !isProtectedFromWalling(xx, yy))
								edges.emplace_back(xx, yy);
					for (size_t i = edges.size(); i > 1; --i) {
						size_t j = rng.below((uint32_t)i);
						std::swap(edges[i - 1], edges[j]);
					}
					bool placed = false;
					for (auto& e : edges) {
						if (!wouldCreateDeadEnd(g, e.first, e.second)) {
							set(g, e.first, e.second, 'W');
							placed = true;
							break;
						}
					}
					if (!placed) set(g, cx, cy, 'W');  // no safe position — forced fallback
				}
			}
		}

		// ---- Phase 17 — connectivity fix (BFS repair) -----------------------------------
		static void phase17_connectivityRepair(std::vector<std::string>& g) {
			static const int dx[4] = { 0, 0, -1, 1 };
			static const int dy[4] = { -1, 1, 0, 0 };

			for (int iter = 0; iter < 30; ++iter) {
				std::vector<std::vector<bool>> reached(kRows, std::vector<bool>(kCols, false));
				std::vector<std::pair<int, int>> queue;
				queue.emplace_back(kPacX, kPacY);
				reached[kPacY][kPacX] = true;
				size_t head = 0;
				while (head < queue.size()) {
					auto [cx, cy] = queue[head++];
					for (int d = 0; d < 4; ++d) {
						int nx = cx + dx[d], ny = cy + dy[d];
						if (!inBounds(nx, ny) || reached[ny][nx]) continue;
						if (!isPassable(g[ny][nx])) continue;
						reached[ny][nx] = true;
						queue.emplace_back(nx, ny);
					}
				}

				int tx = -1, ty = -1;
				for (int y = 0; y < kRows && tx < 0; ++y)
					for (int x = 0; x < kCols; ++x)
						if (isPassable(g[y][x]) && !reached[y][x]) { tx = x; ty = y; break; }
				if (tx < 0) break;  // everything reachable — done

				// BFS from the isolated tile through everything (including WALL) to
				// find the shortest bridge back to the reachable region.
				std::vector<std::vector<bool>> visited(kRows, std::vector<bool>(kCols, false));
				std::vector<std::vector<std::pair<int, int>>> parent(
					kRows, std::vector<std::pair<int, int>>(kCols, { -1, -1 }));
				std::vector<std::pair<int, int>> bq;
				bq.emplace_back(tx, ty);
				visited[ty][tx] = true;
				size_t bhead = 0;
				std::pair<int, int> foundAt = { -1, -1 };
				while (bhead < bq.size()) {
					auto [cx, cy] = bq[bhead++];
					if (reached[cy][cx]) { foundAt = { cx, cy }; break; }
					for (int d = 0; d < 4; ++d) {
						int nx = cx + dx[d], ny = cy + dy[d];
						if (!inBounds(nx, ny) || visited[ny][nx]) continue;
						visited[ny][nx] = true;
						parent[ny][nx] = { cx, cy };
						bq.emplace_back(nx, ny);
					}
				}
				if (foundAt.first < 0) break;  // safety: shouldn't happen on a bounded grid

				int cx = foundAt.first, cy = foundAt.second;
				while (!(cx == tx && cy == ty)) {
					if (g[cy][cx] == 'W') set(g, cx, cy, '.');
					auto p = parent[cy][cx];
					cx = p.first; cy = p.second;
				}
			}
		}

		// ---- Phase 18 — place power pellets ------------------------------------------
		static void phase18_powerPellets(std::vector<std::string>& g, XorShift32& rng) {
			const std::array<std::pair<int, int>, 4> corners = { {{1, 1}, {17, 1}, {1, 19}, {17, 19}} };
			auto absInt = [](int v) { return v < 0 ? -v : v; };

			for (auto& corner : corners) {
				int ax = corner.first, ay = corner.second;
				bool placed = false;
				for (int dist = 0; dist <= 11 && !placed; ++dist) {
					std::vector<std::pair<int, int>> candidates;
					for (int y = ay - dist; y <= ay + dist; ++y) {
						for (int x = ax - dist; x <= ax + dist; ++x) {
							if (std::max(absInt(x - ax), absInt(y - ay)) != dist) continue;  // ring only
							if (get(g, x, y) == '.') candidates.emplace_back(x, y);
						}
					}
					if (!candidates.empty()) {
						auto& pick = candidates[rng.below((uint32_t)candidates.size())];
						set(g, pick.first, pick.second, 'P');
						placed = true;
					}
				}
			}
		}

		// ---- Phase 19 — final re-enforce ------------------------------------------------
		static void phase19_finalReinforce(std::vector<std::string>& g) {
			phase10_ghostHouse(g);
			set(g, 9, 7, '.');
			for (int y = 0; y < kRows; ++y) { set(g, 0, y, 'W'); set(g, kCols - 1, y, 'W'); }
			for (int x = 0; x < kCols; ++x) { set(g, x, 0, 'W'); set(g, x, kRows - 1, 'W'); }
			for (int x = 0; x < kCols; ++x) set(g, x, 12, ' ');
			set(g, 0, 12, ' ');
			set(g, kCols - 1, 12, ' ');
		}

		// ---- Phase 20 — convert stray EMPTY to DOT ---------------------------------------
		static void phase20_strayEmptyToDot(std::vector<std::string>& g) {
			for (int y = 0; y < kRows; ++y) {
				for (int x = 0; x < kCols; ++x) {
					if (g[y][x] != ' ') continue;
					if (y == 12) continue;
					if (x == kPacX && y == kPacY) continue;
					g[y][x] = '.';
				}
			}
		}

		// ---- Visual cleanup — remove tiny floating wall fragments -----------------------
		static void removeFloatingWallFragments(std::vector<std::string>& g, int minComponentSize) {
			static const int dx[4] = { 0, 0, -1, 1 };
			static const int dy[4] = { -1, 1, 0, 0 };
			std::vector<std::vector<bool>> visited(kRows, std::vector<bool>(kCols, false));

			for (int y = 1; y < kRows - 1; ++y) {
				for (int x = 1; x < kCols - 1; ++x) {
					if (g[y][x] != 'W' || visited[y][x]) continue;

					std::vector<std::pair<int, int>> comp;
					std::vector<std::pair<int, int>> stack;
					stack.emplace_back(x, y);
					visited[y][x] = true;
					while (!stack.empty()) {
						auto [cx, cy] = stack.back();
						stack.pop_back();
						comp.emplace_back(cx, cy);
						for (int d = 0; d < 4; ++d) {
							int nx = cx + dx[d], ny = cy + dy[d];
							if (nx < 1 || nx >= kCols - 1 || ny < 1 || ny >= kRows - 1) continue;
							if (visited[ny][nx] || g[ny][nx] != 'W') continue;
							visited[ny][nx] = true;
							stack.emplace_back(nx, ny);
						}
					}
					if ((int)comp.size() < minComponentSize)
						for (auto& t : comp) set(g, t.first, t.second, '.');
				}
			}
		}

		// ---- Phase 21 — add single-width loop connections (more route variety) ---------
		static constexpr int kLoopOpenPercent = 40;  // tune: higher = more loops / more open
		static void phase21_addLoopConnections(std::vector<std::string>& g, XorShift32& rng) {
			for (int y = 1; y < kRows - 1; ++y) {
				if (y == 12) continue;                       // tunnel row — leave alone
				for (int x = 1; x < kCols - 1; ++x) {
					if (g[y][x] != 'W') continue;
					if (isGhostZone(x, y)) continue;         // sacred zone (covers ghost walls too)
					bool up = isOpenTile(get(g, x, y - 1));
					bool down = isOpenTile(get(g, x, y + 1));
					bool left = isOpenTile(get(g, x - 1, y));
					bool right = isOpenTile(get(g, x + 1, y));
					bool horizLink = left && right && !up && !down;  // joins two vertical corridors
					bool vertLink = up && down && !left && !right;   // joins two horizontal corridors
					if ((horizLink || vertLink) && rng.chance(kLoopOpenPercent))
						set(g, x, y, '.');
				}
			}
		}

		// ---- Phase 22 — enforce single-tile-wide corridors -----------------------------
		static int reachableOpenCount(const std::vector<std::string>& g, int sx, int sy) {
			static const int dx[4] = { 0, 0, -1, 1 };
			static const int dy[4] = { -1, 1, 0, 0 };
			if (!isOpenTile(get(g, sx, sy))) return 0;
			std::vector<std::vector<bool>> seen(kRows, std::vector<bool>(kCols, false));
			std::vector<std::pair<int, int>> q;
			q.emplace_back(sx, sy);
			seen[sy][sx] = true;
			size_t head = 0; int count = 0;
			while (head < q.size()) {
				auto [cx, cy] = q[head++];
				++count;
				for (int d = 0; d < 4; ++d) {
					int nx = cx + dx[d], ny = cy + dy[d];
					if (!inBounds(nx, ny) || seen[ny][nx]) continue;
					if (!isOpenTile(g[ny][nx])) continue;
					seen[ny][nx] = true;
					q.emplace_back(nx, ny);
				}
			}
			return count;
		}

		static bool isProtectedFromEnforcement(const std::vector<std::string>& g, int x, int y) {
			if (isProtectedFromWalling(x, y)) return true;   // border corridors + ghost zone
			if (y == 12) return true;                        // tunnel row
			if (x == 9 && y == 7) return true;               // ghost-house access tile
			if (x == kPacX && y == kPacY) return true;       // pacman spawn tile
			if (get(g, x, y) == 'P') return true;            // power pellet
			return false;
		}

		static void phase22_enforceSingleWidth(std::vector<std::string>& g, XorShift32& rng) {
			(void)rng;
			for (int pass = 0; pass < 8; ++pass) {
				bool anyWalled = false;
				int baseReach = reachableOpenCount(g, kPacX, kPacY);
				for (int y = 0; y < kRows - 1; ++y) {
					for (int x = 0; x < kCols - 1; ++x) {
						// detect a 2x2 window with top-left at (x, y) that is fully open
						if (!(isOpenTile(get(g, x, y)) && isOpenTile(get(g, x + 1, y)) &&
							isOpenTile(get(g, x, y + 1)) && isOpenTile(get(g, x + 1, y + 1))))
							continue;

						const std::pair<int, int> corners[4] = {
							{x, y}, {x + 1, y}, {x, y + 1}, {x + 1, y + 1} };
						int bestIdx = -1, bestScore = -1;
						for (int c = 0; c < 4; ++c) {
							int cx = corners[c].first, cy = corners[c].second;
							if (isProtectedFromEnforcement(g, cx, cy)) continue;
							if (wouldCreateDeadEnd(g, cx, cy)) continue;
							// connectivity guard: walling must drop exactly the walled tile
							char save = g[cy][cx];
							g[cy][cx] = 'W';
							int after = reachableOpenCount(g, kPacX, kPacY);
							g[cy][cx] = save;
							if (after != baseReach - 1) continue;   // would isolate something
							// prefer corners that merge into existing walls (no floating speck)
							int wallNbrs = (get(g, cx - 1, cy) == 'W') + (get(g, cx + 1, cy) == 'W')
								+ (get(g, cx, cy - 1) == 'W') + (get(g, cx, cy + 1) == 'W');
							if (wallNbrs > bestScore) { bestScore = wallNbrs; bestIdx = c; }
						}
						if (bestIdx >= 0) {
							set(g, corners[bestIdx].first, corners[bestIdx].second, 'W');
							--baseReach;              // one reachable open tile removed
							anyWalled = true;
						}
					}
				}
				if (!anyWalled) break;
			}
		}

		static bool isBorderCell(int x, int y) { return x == 0 || x == kCols - 1 || y == 0 || y == kRows - 1; }

		static bool isGhostWallCell(int x, int y) {
			if (y == 8 && x >= 6 && x <= 12) return true;
			if (y == 11 && x >= 6 && x <= 12) return true;
			if (x == 6 && y >= 8 && y <= 11) return true;
			if (x == 12 && y >= 8 && y <= 11) return true;
			return false;
		}

		static char borderGlyph(int x, int y) {
			bool top = (y == 0), bottom = (y == kRows - 1), left = (x == 0), right = (x == kCols - 1);
			if (top && left) return '!';
			if (top && right) return '#';
			if (bottom && left) return '%';
			if (bottom && right) return '^';
			if (top || bottom) return '@';
			return '$';
		}

		static char ghostHouseGlyph(int x, int y) {
			if (x == 6 && y == 8) return '!';
			if (x == 12 && y == 8) return '#';
			if (x == 6 && y == 11) return '%';
			if (x == 12 && y == 11) return '^';
			if (y == 8 || y == 11) return '@';
			return '$';
		}

		// Classifies an interior generated wall tile into one of the six
		// double-line "box" wall glyphs based on which orthogonal neighbors are
		// also walls — an autotile pass.
		static char autotileWall(const std::vector<std::string>& g, int x, int y) {
			auto wallAt = [&](int xx, int yy) {
				if (!inBounds(xx, yy)) return true;
				return g[yy][xx] == 'W';
				};
			bool up = wallAt(x, y - 1), down = wallAt(x, y + 1), left = wallAt(x - 1, y), right = wallAt(x + 1, y);
			if (left && right && !up && !down) return '@';
			if (up && down && !left && !right) return '$';
			if (right && down && !left && !up) return '!';
			if (left && down && !right && !up) return '#';
			if (right && up && !left && !down) return '%';
			if (left && up && !right && !down) return '^';
			int vCount = (up ? 1 : 0) + (down ? 1 : 0);
			int hCount = (left ? 1 : 0) + (right ? 1 : 0);
			if (vCount > hCount) return '$';
			if (hCount > vCount) return '@';
			if (vCount > 0) return '$';  // tie (e.g. a 4-way crossing) — pick an axis
			return '@';                   // fully isolated — shouldn't occur post-cleanup
		}

		static std::vector<std::string> toRenderChars(const std::vector<std::string>& g) {
			std::vector<std::string> out(kRows, std::string(kCols, ' '));
			for (int y = 0; y < kRows; ++y) {
				for (int x = 0; x < kCols; ++x) {
					char c = g[y][x];
					char rc = ' ';
					switch (c) {
					case 'H': rc = ' '; break;
					case 'D': rc = '-'; break;
					case '.': rc = '*'; break;
					case 'P': rc = '0'; break;
					case ' ': rc = ' '; break;
					case 'W':
						if (isBorderCell(x, y)) rc = borderGlyph(x, y);
						else if (isGhostWallCell(x, y)) rc = ghostHouseGlyph(x, y);
						else rc = autotileWall(g, x, y);
						break;
					default: rc = ' '; break;
					}
					out[y][x] = rc;
				}
			}
			// Spawn markers, placed last so they overwrite the underlying tile.
			out[kPacY][kPacX] = 'o';
			out[9][8] = 'b';
			out[9][9] = 'i';
			out[9][10] = 'p';
			out[10][9] = 'c';
			return out;
		}
	};

}  // namespace

void MapGenerator::generate(uint32_t seed) {
	if (seed == 0) {
		std::random_device rd;
		seed = rd();
	}
	grid_ = MazeGen::generate(seed);
}

char MapGenerator::at(int col, int row) const {
	if (row < 0 || row >= static_cast<int>(grid_.size())) return ' ';
	if (col < 0 || col >= static_cast<int>(grid_[row].size())) return ' ';
	return grid_[row][col];
}
