# **Score & HighScore Refactoring — OOP Inheritance + Centralized File I/O**

## **What Changed and Why**

Before this refactor, every game had its **own copy** of high-score logic — identical `int highScore` variables, manual `if (score > highScore)` checks, and scattered `saveHighScore()` calls. Rocket Shooter and Pac-Man even used **raw `std::ifstream`/`std::ofstream`** instead of the centralized `FileManager`/`GameDataManager`. Pac-Man had **no high score persistence** at all.

The fix: create a **`Score` base class** and a **`HighScore` derived class** (inheritance + polymorphism), then wire every game to use them.

---

## **1. The New Classes**

### **`Score` (Base Class)** — `include/common/HighScore.h`

A simple base that holds a score integer and provides virtual methods:

| Method | What it does |
|--------|-------------|
| `reset()` | Sets score to 0 |
| `add(points)` | Adds points to score |
| `set(value)` | Overwrites score |
| `get()` | Returns current score |
| `operator>(rhs)` | Compares score to another value |

### **`HighScore` (Derived Class)** — `include/common/HighScore.h`

Inherits from `Score`, **overrides** `set()` to auto-save when a new record is beaten:

```cpp
class HighScore : public Score {
private:
    corezone::GameDataManager& gameData_;
public:
    HighScore(corezone::GameDataManager& gameData);
    void load();              // reads from disk via GameDataManager
    void save();              // writes to disk via GameDataManager
    bool isNewHighScore(int currentScore) const;
    void set(int value) override;  // only saves if value > current score
};
```

The **`override`** keyword means the compiler checks that `set()` truly matches the base class signature — if the base changes, this won't compile silently.

---

## **2. How HighScore Works**

```
Game starts
    ↓
HighScore constructor calls load()
    ↓
load() asks GameDataManager: "what's the saved high score?"
    ↓
GameDataManager reads from config/<GAME>_highscore.sav
    ↓
Score is stored in score_ (inherited from Score)
    ↓
During gameplay: highScoreObj.set(newScore) is called
    ↓
set() checks: is newScore > score_? (inherited compare)
    ↓
If YES: score_ = newScore, then save() writes to disk
    ↓
If NO:  nothing happens (old record stands)
```

---

## **3. File-by-File Changes**

### **New files created:**

| File | Purpose |
|------|---------|
| `include/common/HighScore.h` | `Score` base class + `HighScore` derived class declarations |
| `src/common/HighScore.cpp` | `HighScore` implementation (load, save, isNewHighScore, set) |

Both auto-included in the build via CMake's `GLOB_RECURSE`.

### **Rocket Shooter — before vs after**

| Before | After |
|--------|-------|
| `#include <fstream>` | Removed |
| `static const inline std::string HIGH_SCORE_FILE = "..."` | Removed |
| `void loadHighScore()` — raw `std::ifstream` | Removed entirely |
| `void saveHighScore()` — raw `std::ofstream` | Removed entirely |
| `int highScore = 0;` member | `std::unique_ptr<HighScore> highScore;` |
| — | `corezone::GameDataManager gameData;` |
| `RocketShooterGame(sf::RenderWindow& win)` | `RocketShooterGame(sf::RenderWindow& win, corezone::FileManager& fileManager)` |
| `if (score > highScore) { ... saveHighScore(); }` | `highScore->set(score);` |
| `std::to_string(highScore)` in HUD | `std::to_string(highScore->get())` |
| `runRocketShooter(window)` | `runRocketShooter(window, fileManager)` |

### **Pac-Man — before vs after**

| Before | After |
|--------|-------|
| `#include <fstream>` | Removed |
| `explicit Pacman(sf::RenderWindow& window)` | `explicit Pacman(sf::RenderWindow& window, corezone::FileManager& fileManager)` |
| — | `corezone::FileManager* fileManager_;` |
| — | `corezone::GameDataManager gameData_;` |
| — | `HighScore highScore_;` |
| `saveContinueData()` — raw `std::ofstream` | `gameData_.saveGameState(ss.str())` |
| `loadContinueData()` — raw `std::ifstream` | `gameData_.loadGameState(gameState)` then parse string |
| `saveSoundSettings()` — raw `std::ofstream` | `fileManager_->setVolumeData()` + `saveAllVolumeData()` |
| `loadSoundSettings()` — raw `std::ifstream` | `fileManager_->loadAllVolumeData()` + `getVolumeData()` |
| No high score at all | `highScore_.set(score_)` on game over |
| Not displayed in HUD | `highScore_.get()` displayed in `drawHud()` |
| `runPacMan(window)` | `runPacMan(window, fileManager)` |

### **Snake — before vs after**

| Before | After |
|--------|-------|
| `int highScore = 0; gameData.getHighScore(highScore);` | `HighScore highScoreObj(gameData);` |
| `if (score > highScore) { highScore = score; gameData.saveHighScore(highScore); }` | `highScoreObj.set(score);` |
| `std::to_string(highScore)` in HUD | `std::to_string(highScoreObj.get())` |

### **Flappy Bird — before vs after**

| Before | After |
|--------|-------|
| `int highScore = 0; gameData.getHighScore(highScore);` | `HighScore highScoreObj(gameData);` |
| `if (score > highScore) { ... highScore = score; gameData.saveHighScore(highScore); }` | `if (highScoreObj.isNewHighScore(score)) { ... highScoreObj.set(score); }` |
| All `std::to_string(highScore)` in display text | `std::to_string(highScoreObj.get())` |

### **Dino Run — before vs after**

| Before | After |
|--------|-------|
| `int highscore = 0; gameData.getHighScore(highscore);` | `HighScore highScoreObj(gameData);` |
| `if (score > highscore) { highscore = score; gameData.saveHighScore(highscore); }` | `if (highScoreObj.isNewHighScore(static_cast<int>(score))) { highScoreObj.set(static_cast<int>(score)); }` |
| `std::to_string(highscore)` in HUD | `std::to_string(highScoreObj.get())` |

### **main.cpp — all changes**

| Before | After |
|--------|-------|
| `void runRocketShooter(sf::RenderWindow& window);` | `void runRocketShooter(sf::RenderWindow& window, corezone::FileManager& fileManager);` |
| `runRocketShooter(window)` | `runRocketShooter(window, fileManager)` |
| `runPacMan(window)` (x2 call sites) | `runPacMan(window, fileManager)` (x2 call sites) |

---

## **4. Inheritance Diagram**

```
Score  (base class)
│
│  protected: int score_
│  virtual reset(), add(), set(), get(), operator>()
│
└──── HighScore  (derived class)
     │
     │  private: corezone::GameDataManager& gameData_
     │
     │  void set(int value) override  ← function overriding!
     │    └─ only writes to disk if value > current score
     │
     │  void load()   ← reads from GameDataManager
     │  void save()   ← writes via GameDataManager
     │  bool isNewHighScore(currentScore)
     │
     └─ Used by: Snake, Flappy Bird, Dino Run,
                 Rocket Shooter, Pac-Man
```

---

## **5. Key OOP Concepts Demonstrated**

### **Inheritance**
`HighScore` inherits `score_` and all public methods from `Score`. Every game gets the same base behavior without redefining it.

### **Polymorphism**
Any function expecting a `Score&` can accept a `HighScore&`. The virtual method table ensures `set()` calls `HighScore::set()` (with save logic) instead of `Score::set()`.

### **Function Overriding**
`HighScore::set(int)` uses the `override` specifier to tell the compiler "I mean to replace the base class version." If the base signature ever changes, the compiler catches the mismatch.

---

## **6. Data Flow Summary**

```
main.cpp
  │
  ├─ Creates FileManager
  │
  ├─ Passes FileManager to each game's entry function
  │
  ├─ runSnake(window, fileManager)
  │   └─ Creates GameDataManager(fileManager, "SNAKE")
  │       └─ Creates HighScore(gameData)
  │           ├─ load() on construction
  │           └─ set() → save() on new record
  │
  ├─ runFlappyBird(window, fileManager)
  │   └─ Same pattern with "FLAPPYBIRD"
  │
  ├─ RunDino(window, fileManager)
  │   └─ Same pattern with "DINO RUN"
  │
  ├─ runRocketShooter(window, fileManager)
  │   └─ GameDataManager(fileManager, "ROCKETSHOOTER")
  │       └─ unique_ptr<HighScore>
  │
  └─ runPacMan(window, fileManager)
      └─ GameDataManager(fileManager, "PACMAN")
          └─ HighScore as member
```

Saved files live in `config/`:
```
config/
  ├── SNAKE_highscore.sav
  ├── FLAPPYBIRD_highscore.sav
  ├── DINO RUN_highscore.sav
  ├── ROCKETSHOOTER_highscore.sav
  ├── PACMAN_highscore.sav
  ├── PACMAN_state.sav          (continue data)
  └── audio.cfg                 (Pac-Man volume settings)
```

---

## **7. Why This Is Better**

| Problem | Before | After |
|---------|--------|-------|
| Code duplication | 5 copies of identical high-score logic | 1 `HighScore` class, 5 usages |
| Raw file I/O | Rocket Shooter + Pac-Man used `ifstream`/`ofstream` directly | All games go through `GameDataManager` → `FileManager` |
| Missing high scores | Pac-Man had no high score at all | Pac-Man saves/loads high score like every other game |
| Tight coupling | Each game managed its own file paths | Centralized path resolution via `FileManager` |
| No polymorphism | All score types were plain `int` | `Score` base enables future extensions (e.g., `TimedScore`, `MultiplayerScore`) |
