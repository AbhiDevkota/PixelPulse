#ifndef GRID_H
#define GRID_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

/*
* =====================================================================
*  DebugGrid — In-Game Coordinate Debug Tool
* =====================================================================
*
*  CONTROLS:
*  ---------
*  F5          → Open / Close debug overlay (pauses game)
*  +  / -      → Increase / Decrease cell size
*  M           → Toggle live mouse coordinate display
*  ESC         → Close debug overlay
*
*  MOUSE (while debug is open):
*  ----------------------------
*  Hover       → Shows live X,Y in top bar
*  Click+Drag  → Select a rectangular region
*                Shows: start, end, width, height
*                Copies to clipboard automatically on mouse release
*
*  CONSOLE OUTPUT on drag release:
*  --------------------------------
*  [GRID] Selected Region:
*    Start : (120, 300)
*    End   : (450, 580)
*    Size  : 330 x 280
*    setPosition({ 120.f, 300.f });
*    setSize({ 330.f, 280.f });
*
*  HOW TO PLUG IN:
*  ---------------
*  1. Cricket.h private:
*       DebugGrid debugGrid;
*
*  2. Constructor end:
*       debugGrid.init(window, font_path);
*
*  3. handleInput(event, window) — top of function:
*       debugGrid.handleInput(event, window);
*       if (debugGrid.isOpen()) return;
*
*  4. update(dt, window) — top of function:
*       debugGrid.update(window);
*       if (debugGrid.isOpen()) return;
*
*  5. draw(window) — LAST line:
*       debugGrid.draw(window);
* =====================================================================
*/

struct SelectedRegion {
    sf::Vector2i start{ 0, 0 };
    sf::Vector2i end{ 0, 0 };
    bool active = false;

    int left()   const { return std::min(start.x, end.x); }
    int top()    const { return std::min(start.y, end.y); }
    int width()  const { return std::abs(end.x - start.x); }
    int height() const { return std::abs(end.y - start.y); }
};

class DebugGrid {
public:
    DebugGrid();

    void init(sf::RenderWindow& window, const std::string& fontPath = "fonts/regular.ttf");

    void handleInput(const sf::Event& event, sf::RenderWindow& window);

    void update(sf::RenderWindow& window);

    void draw(sf::RenderWindow& window);

    bool isOpen() const;

private:
    void buildGrid(sf::RenderWindow& window);
    void updateMouseLabel(sf::RenderWindow& window);
    void copyToClipboard(sf::RenderWindow& window, const std::string& text);
    void printSelection();
    void buildInfoPanel();
    std::string buildSelectionString();

    bool open = false;
    bool showMouse = true;
    int  cellSize = 50;
    int  minCellSize = 25;
    int  maxCellSize = 200;

    bool dragging = false;
    SelectedRegion selection;

    sf::Font font;
    bool fontLoaded = false;

    sf::Vector2u windowSize;

    sf::VertexArray gridLines;
    std::vector<sf::Text> coordLabels;

    sf::RectangleShape selectionRect;
    sf::RectangleShape topBar;
    sf::RectangleShape infoPanel;
    sf::RectangleShape overlay;

    sf::Text mouseCoordText{ font };
    sf::Text hintText{ font };
    sf::Text infoText{ font };
    sf::Text copiedText{ font };

    float copiedTimer = 0.f;
    float copiedDuration = 1.5f;
};

#endif