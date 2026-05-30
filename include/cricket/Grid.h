#ifndef GRID_H
#define GRID_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

/*
* DebugGrid - A coordinate overlay for the Cricket game
*
* USAGE:
*   1. Add "DebugGrid debugGrid;" to CricketGame private section in Cricket.h
*   2. Call debugGrid.init(window) in CricketGame constructor
*   3. Call debugGrid.handleInput(event) in CricketGame::handleInput()
*   4. Call debugGrid.draw(window) as the LAST line in CricketGame::draw()
*
* CONTROLS (in-game):
*   G        → Toggle grid on/off
*   +/-      → Increase/decrease grid cell size (default 50px)
*   M        → Toggle mouse coordinate display
*
* HOW TO USE FOR POSITIONING:
*   Run the game, press G to show grid.
*   Hover mouse over any point (e.g. where stumps are in background).
*   The top-left shows exact X,Y coordinates → use those in your code.
*/

class DebugGrid {
public:
    DebugGrid();

    void init(sf::RenderWindow& window, const std::string& fontPath = "fonts/regular.ttf");

    void handleInput(const sf::Event& event, sf::RenderWindow& window);

    void update(sf::RenderWindow& window); //call every frame to update mouse coords

    void draw(sf::RenderWindow& window);

    bool isVisible() const;

private:
    void buildGrid(sf::RenderWindow& window);
    void updateCoordLabel(sf::RenderWindow& window);

    bool visible = false;           //G to toggle
    bool showMouseCoords = true;    //M to toggle
    int cellSize = 50;              //pixels per cell, +/- to change
    int minCellSize = 25;
    int maxCellSize = 200;

    sf::Font font;
    bool fontLoaded = false;

    //Grid lines stored as vertex array for performance
    sf::VertexArray gridLines;

    //Axis labels (every other cell to avoid clutter)
    std::vector<sf::Text> labels;

    // Mouse coordinate display (top-left corner)
    sf::Text coordText{ font };

    //Semi-transparent overlay so grid is readable over background
    sf::RectangleShape overlay;

    sf::Vector2u windowSize;
};

#endif