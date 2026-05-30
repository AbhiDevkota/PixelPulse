#include "cricket/Grid.h"
#include <iostream>
#include <sstream>
#include <iomanip>



static std::string coordStr(int x, int y) {
    return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
}


DebugGrid::DebugGrid() {}

void DebugGrid::init(sf::RenderWindow& window, const std::string& fontPath) {
    windowSize = window.getSize();

    fontLoaded = font.openFromFile(fontPath);
    if (!fontLoaded)
        std::cerr << "[GRID] Warning: font not loaded from " << fontPath << std::endl;

    // ── Overlay ──────────────────────────────
    overlay.setSize({ static_cast<float>(windowSize.x), static_cast<float>(windowSize.y) });
    overlay.setFillColor(sf::Color(0, 0, 0, 80));

    // ── Top bar ──────────────────────────────
    topBar.setSize({ static_cast<float>(windowSize.x), 36.f });
    topBar.setFillColor(sf::Color(20, 20, 20, 220));
    topBar.setPosition({ 0.f, 0.f });

    // ── Info panel (bottom) ──────────────────
    infoPanel.setSize({ static_cast<float>(windowSize.x), 90.f });
    infoPanel.setFillColor(sf::Color(15, 15, 35, 230));
    infoPanel.setPosition({ 0.f, static_cast<float>(windowSize.y) - 90.f });

    // ── Selection rectangle ──────────────────
    selectionRect.setFillColor(sf::Color(100, 180, 255, 50));
    selectionRect.setOutlineColor(sf::Color(100, 200, 255, 255));
    selectionRect.setOutlineThickness(2.f);

    if (fontLoaded) {
        // Mouse coord text
        mouseCoordText = sf::Text(font);
        mouseCoordText.setCharacterSize(16);
        mouseCoordText.setFillColor(sf::Color(255, 255, 100));
        mouseCoordText.setPosition({ 10.f, 8.f });
        mouseCoordText.setString("X: 0  Y: 0");

        // Hint text (right side of top bar)
        hintText = sf::Text(font);
        hintText.setCharacterSize(14);
        hintText.setFillColor(sf::Color(160, 160, 160));
        hintText.setString("[F5/ESC] Close   [+/-] Cell size   [M] Coords   [Drag] Select region");

        // Position hint text to right side
        sf::FloatRect hb = hintText.getLocalBounds();
        hintText.setPosition({ static_cast<float>(windowSize.x) - hb.size.x - 12.f, 10.f });

        // Info panel text
        infoText = sf::Text(font);
        infoText.setCharacterSize(16);
        infoText.setFillColor(sf::Color(180, 220, 255));
        infoText.setPosition({ 12.f, static_cast<float>(windowSize.y) - 85.f });
        infoText.setString("Drag on the grid to select a region. Coordinates will appear here.");

        // "Copied!" flash text
        copiedText = sf::Text(font);
        copiedText.setCharacterSize(22);
        copiedText.setFillColor(sf::Color(80, 255, 120));
        copiedText.setOutlineColor(sf::Color::Black);
        copiedText.setOutlineThickness(2.f);
        copiedText.setString("✔ Copied to clipboard!");
        sf::FloatRect cb = copiedText.getLocalBounds();
        copiedText.setOrigin({ cb.size.x / 2.f, cb.size.y / 2.f });
        copiedText.setPosition({
            static_cast<float>(windowSize.x) / 2.f,
            static_cast<float>(windowSize.y) / 2.f - 40.f
            });
    }

    buildGrid(window);

    std::cout << "[GRID] Debug grid initialized. Press F5 in-game to open." << std::endl;
}

void DebugGrid::buildGrid(sf::RenderWindow& window) {
    windowSize = window.getSize();
    gridLines.clear();
    coordLabels.clear();
    gridLines.setPrimitiveType(sf::PrimitiveType::Lines);

    unsigned int w = windowSize.x;
    unsigned int h = windowSize.y;

    // Vertical lines
    for (unsigned int x = 0; x <= w; x += cellSize) {
        bool major = (x % 100 == 0);
        sf::Color col = major ? sf::Color(255, 255, 255, 110) : sf::Color(255, 255, 255, 38);

        sf::Vertex a, b;
        a.position = { static_cast<float>(x), 0.f };
        b.position = { static_cast<float>(x), static_cast<float>(h) };
        a.color = b.color = col;
        gridLines.append(a);
        gridLines.append(b);
    }

    // Horizontal lines
    for (unsigned int y = 0; y <= h; y += cellSize) {
        bool major = (y % 100 == 0);
        sf::Color col = major ? sf::Color(255, 255, 255, 110) : sf::Color(255, 255, 255, 38);

        sf::Vertex a, b;
        a.position = { 0.f, static_cast<float>(y) };
        b.position = { static_cast<float>(w), static_cast<float>(y) };
        a.color = b.color = col;
        gridLines.append(a);
        gridLines.append(b);
    }

    // Labels at every 100px intersection
    if (fontLoaded) {
        for (unsigned int x = 0; x <= w; x += 100) {
            for (unsigned int y = 0; y <= h; y += 100) {
                sf::Text lbl(font);
                lbl.setCharacterSize(12);
                lbl.setFillColor(sf::Color(255, 210, 60, 200));
                lbl.setOutlineColor(sf::Color(0, 0, 0, 160));
                lbl.setOutlineThickness(1.f);
                lbl.setString(std::to_string(x) + "," + std::to_string(y));
                lbl.setPosition({ static_cast<float>(x) + 3.f, static_cast<float>(y) + 3.f });
                coordLabels.push_back(lbl);
            }
        }
    }
}

void DebugGrid::handleInput(const sf::Event& event, sf::RenderWindow& window) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::F5) {
            open = !open;
            if (open)  std::cout << "[GRID] Debug overlay opened." << std::endl;
            else       std::cout << "[GRID] Debug overlay closed." << std::endl;
            return;
        }

        if (!open) return;

        // ESC closes
        if (key->code == sf::Keyboard::Key::Escape) {
            open = false;
            std::cout << "[GRID] Debug overlay closed." << std::endl;
            return;
        }

        // +/- cell size
        if (key->code == sf::Keyboard::Key::Equal) {
            cellSize = std::min(cellSize + 25, maxCellSize);
            buildGrid(window);
            std::cout << "[GRID] Cell size: " << cellSize << "px" << std::endl;
        }
        if (key->code == sf::Keyboard::Key::Hyphen) {
            cellSize = std::max(cellSize - 25, minCellSize);
            buildGrid(window);
            std::cout << "[GRID] Cell size: " << cellSize << "px" << std::endl;
        }

        // M toggle mouse coords
        if (key->code == sf::Keyboard::Key::M) {
            showMouse = !showMouse;
        }
    }

    if (!open) return;

	//Mouse Drag Selection
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            dragging = true;
            selection.start = { mb->position.x, mb->position.y };
            selection.end = { mb->position.x, mb->position.y };
            selection.active = false;
        }
    }

    if (const auto* mm = event.getIf<sf::Event::MouseMoved>()) {
        if (dragging) {
            selection.end = { mm->position.x, mm->position.y };
        }
    }

    if (const auto* mb = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mb->button == sf::Mouse::Button::Left && dragging) {
            dragging = false;
            selection.end = { mb->position.x, mb->position.y };
            selection.active = true;

            printSelection();
            std::string clip = buildSelectionString();
            copyToClipboard(window, clip);
            copiedTimer = copiedDuration;
            buildInfoPanel();
        }
    }
}

void DebugGrid::update(sf::RenderWindow& window) {
    if (!open) return;

    // Live mouse coords
    if (showMouse && fontLoaded) {
        updateMouseLabel(window);
    }

    // Fade the "Copied!" text
    if (copiedTimer > 0.f) {
        // We don't have dt here so we use a rough fixed step
        // For accuracy, pass dt — but this works fine visually
        copiedTimer -= 0.016f;
        float alpha = std::max(0.f, (copiedTimer / copiedDuration) * 255.f);
        sf::Color c = copiedText.getFillColor();
        c.a = static_cast<uint8_t>(alpha);
        copiedText.setFillColor(c);
    }

    // Update live selection rect while dragging
    if (dragging) {
        selectionRect.setPosition({
            static_cast<float>(selection.left()),
            static_cast<float>(selection.top())
            });
        selectionRect.setSize({
            static_cast<float>(selection.width()),
            static_cast<float>(selection.height())
            });
    }
}

void DebugGrid::updateMouseLabel(sf::RenderWindow& window) {
    sf::Vector2i mp = sf::Mouse::getPosition(window);
    int mx = std::max(0, std::min(mp.x, static_cast<int>(windowSize.x)));
    int my = std::max(0, std::min(mp.y, static_cast<int>(windowSize.y)));

    mouseCoordText.setString(
        "X: " + std::to_string(mx) +
        "  Y: " + std::to_string(my) +
        "  |  Cell: " + std::to_string(cellSize) + "px"
    );
}

std::string DebugGrid::buildSelectionString() {
    std::ostringstream ss;
    ss << "[GRID] Selected Region:\n"
        << "  Start : " << coordStr(selection.left(), selection.top()) << "\n"
        << "  End   : " << coordStr(selection.left() + selection.width(), selection.top() + selection.height()) << "\n"
        << "  Size  : " << selection.width() << " x " << selection.height() << "\n"
        << "  setPosition({ " << selection.left() << ".f, " << selection.top() << ".f });\n"
        << "  setSize({ " << selection.width() << ".f, " << selection.height() << ".f });";
    return ss.str();
}

void DebugGrid::printSelection() {
    std::cout << "\n" << buildSelectionString() << "\n" << std::endl;
}

void DebugGrid::buildInfoPanel() {
    if (!fontLoaded) return;
    std::ostringstream ss;
    ss << "Start: " << coordStr(selection.left(), selection.top())
        << "   End: " << coordStr(selection.left() + selection.width(), selection.top() + selection.height())
        << "   Size: " << selection.width() << " x " << selection.height() << "\n"
        << "setPosition({ " << selection.left() << ".f, " << selection.top() << ".f });   "
        << "setSize({ " << selection.width() << ".f, " << selection.height() << ".f });";
    infoText.setString(ss.str());
}

void DebugGrid::copyToClipboard(sf::RenderWindow& window, const std::string& text) {
    sf::Clipboard::setString(text);
    std::cout << "[GRID] Copied to clipboard." << std::endl;
}

void DebugGrid::draw(sf::RenderWindow& window) {
    if (!open) return;

    // 1. Dark overlay
    window.draw(overlay);

    // 2. Grid lines
    window.draw(gridLines);

    // 3. Coordinate labels
    for (auto& lbl : coordLabels)
        window.draw(lbl);

    // 4. Drag selection rectangle
    if (dragging || selection.active) {
        window.draw(selectionRect);
    }

    // 5. Top bar
    window.draw(topBar);
    if (fontLoaded) {
        if (showMouse) window.draw(mouseCoordText);
        window.draw(hintText);
    }

    // 6. Bottom info panel
    window.draw(infoPanel);
    if (fontLoaded) window.draw(infoText);

    // 7. "Copied!" flash
    if (copiedTimer > 0.f && fontLoaded)
        window.draw(copiedText);
}


bool DebugGrid::isOpen() const {
    return open;
}