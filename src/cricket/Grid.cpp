#include "cricket/Grid.h"
#include <string>

DebugGrid::DebugGrid() {
    //nothing until init() is called
}

void DebugGrid::init(sf::RenderWindow& window, const std::string& fontPath) {
    windowSize = window.getSize();

    //Load font - soft fail, grid lines still work without font
    if (font.openFromFile(fontPath)) {
        fontLoaded = true;
    }

    //Semi-transparent dark overlay so grid is readable on bright backgrounds
    overlay.setSize({
        static_cast<float>(windowSize.x),
        static_cast<float>(windowSize.y)
        });
    overlay.setFillColor(sf::Color(0, 0, 0, 60)); //very subtle, just darkens slightly

    //Mouse coord label — top left corner
    if (fontLoaded) {
        coordText = sf::Text(font);
        coordText.setCharacterSize(18);
        coordText.setFillColor(sf::Color(255, 255, 0)); //yellow, easy to read
        coordText.setOutlineColor(sf::Color::Black);
        coordText.setOutlineThickness(1.5f);
        coordText.setPosition({ 10.f, 10.f });
        coordText.setString("X: 0  Y: 0");
    }

    buildGrid(window);
}

void DebugGrid::buildGrid(sf::RenderWindow& window) {
    windowSize = window.getSize();
    gridLines.clear();
    labels.clear();

    gridLines.setPrimitiveType(sf::PrimitiveType::Lines);

    unsigned int w = windowSize.x;
    unsigned int h = windowSize.y;

    //Vertical lines
    for (unsigned int x = 0; x <= w; x += cellSize) {
        sf::Vertex top, bottom;
        top.position = { static_cast<float>(x), 0.f };
        bottom.position = { static_cast<float>(x), static_cast<float>(h) };

        //Every 100px line is brighter
        bool isMajor = (x % 100 == 0);
        sf::Color lineColor = isMajor
            ? sf::Color(255, 255, 255, 120)
            : sf::Color(255, 255, 255, 45);

        top.color = lineColor;
        bottom.color = lineColor;

        gridLines.append(top);
        gridLines.append(bottom);
    }

    //Horizontal lines
    for (unsigned int y = 0; y <= h; y += cellSize) {
        sf::Vertex left, right;
        left.position = { 0.f, static_cast<float>(y) };
        right.position = { static_cast<float>(w), static_cast<float>(y) };

        bool isMajor = (y % 100 == 0);
        sf::Color lineColor = isMajor
            ? sf::Color(255, 255, 255, 120)
            : sf::Color(255, 255, 255, 45);

        left.color = lineColor;
        right.color = lineColor;

        gridLines.append(left);
        gridLines.append(right);
    }

    //Coordinate labels at every 100px intersection
    if (fontLoaded) {
        for (unsigned int x = 0; x <= w; x += 100) {
            for (unsigned int y = 0; y <= h; y += 100) {
                sf::Text label(font);
                label.setCharacterSize(13);
                label.setFillColor(sf::Color(255, 220, 80, 200));
                label.setOutlineColor(sf::Color(0, 0, 0, 180));
                label.setOutlineThickness(1.f);
                label.setString(std::to_string(x) + "," + std::to_string(y));
                label.setPosition({
                    static_cast<float>(x) + 3.f,
                    static_cast<float>(y) + 2.f
                    });
                labels.push_back(label);
            }
        }
    }
}

void DebugGrid::handleInput(const sf::Event& event, sf::RenderWindow& window) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {

        //G → toggle grid
        if (key->code == sf::Keyboard::Key::G) {
            visible = !visible;
        }

        //M → toggle mouse coord display
        if (key->code == sf::Keyboard::Key::M) {
            showMouseCoords = !showMouseCoords;
        }

        //+ → increase cell size
        if (key->code == sf::Keyboard::Key::Equal) { // = key (same key as +)
            cellSize = std::min(cellSize + 25, maxCellSize);
            buildGrid(window);
        }

        //- → decrease cell size
        if (key->code == sf::Keyboard::Key::Hyphen) {
            cellSize = std::max(cellSize - 25, minCellSize);
            buildGrid(window);
        }
    }
}

void DebugGrid::update(sf::RenderWindow& window) {
    if (!visible || !showMouseCoords || !fontLoaded) return;
    updateCoordLabel(window);
}

void DebugGrid::updateCoordLabel(sf::RenderWindow& window) {
    sf::Vector2i mousePixel = sf::Mouse::getPosition(window);

    // Clamp to window bounds
    int mx = std::max(0, std::min(mousePixel.x, static_cast<int>(windowSize.x)));
    int my = std::max(0, std::min(mousePixel.y, static_cast<int>(windowSize.y)));

    coordText.setString(
        "X: " + std::to_string(mx) +
        "  Y: " + std::to_string(my) +
        "  |  Cell: " + std::to_string(cellSize) + "px" +
        "  |  [G] hide  [+/-] zoom  [M] coords"
    );
}

void DebugGrid::draw(sf::RenderWindow& window) {
    if (!visible) return;

    //1. Subtle overlay
    window.draw(overlay);

    //2. Grid lines
    window.draw(gridLines);

    //3. Coordinate labels at intersections
    for (auto& label : labels) {
        window.draw(label);
    }

    //4. Mouse position display
    if (showMouseCoords && fontLoaded) {
        window.draw(coordText);
    }
}

bool DebugGrid::isVisible() const {
    return visible;
}