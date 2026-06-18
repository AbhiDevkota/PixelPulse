#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>

// A simple structure to hold the positions of our dynamic objects
struct GridObject {
    sf::Vector2i gridPos;
    sf::Vector2f visualPos;
};

void runRocketShooter(sf::RenderWindow& window) {
    // Seed random for obstacle spawning
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    const int CELL_SIZE = 32;
    auto size = window.getSize();
    const int COLS = size.x / CELL_SIZE;
    const int ROWS = size.y / CELL_SIZE;


    // --- Player Setup ---
    sf::RectangleShape rocket({ static_cast<float>(2*CELL_SIZE), static_cast<float>(2*CELL_SIZE) });
    rocket.setFillColor(sf::Color::Red);
    sf::Vector2i rocketGridPos = { COLS / 2, ROWS - 2 };
    sf::Vector2f rocketVisualPos = { static_cast<float>(rocketGridPos.x * CELL_SIZE), static_cast<float>(rocketGridPos.y * CELL_SIZE) };

    // --- Obstacle Setup ---
    sf::RectangleShape obstacleShape({ static_cast<float>(CELL_SIZE), static_cast<float>(CELL_SIZE) });
    obstacleShape.setFillColor(sf::Color::Blue);
    std::vector<GridObject> obstacles;

    // --- Bullet Setup ---
    sf::RectangleShape bulletShape({ static_cast<float>(CELL_SIZE) * 0.2f, static_cast<float>(CELL_SIZE) * 0.6f }); // Slimmer rectangle
    bulletShape.setFillColor(sf::Color::Yellow);
    std::vector<GridObject> bullets;

    // --- Timers & Speeds ---
    sf::Clock deltaClock;
    sf::Clock obstacleSpawnClock;
    sf::Clock gameTickClock; // Handles fixed-interval movement for bullets/obstacles


    //--- Background setup ---
  //loadassets
    sf::Texture spaceTexture;
    spaceTexture.loadFromFile("assets/rocket/starsrocket.png");
    spaceTexture.setRepeated(true);
    sf::Sprite spaceSprite(spaceTexture);
    spaceSprite.setTextureRect(sf::IntRect(
        { 0,0 },
        { (int)size.x, (int)size.y}));

    const float SLIDE_SPEED = 15.0f;       // Smooth interpolation speed
    const float SPAWN_INTERVAL = 1.5f;     // Spawn an obstacle every 1.5 seconds
    const float GAME_TICK_INTERVAL = 0.2f; // Obstacles drop and bullets rise every 0.2 seconds


    while (window.isOpen()) {
        // 1. Event Handling
        while (auto e = window.pollEvent()) {
            if (e->is<sf::Event::Closed>()) {
                window.close();
            }

            if (auto keyPressed = e->getIf<sf::Event::KeyPressed>()) {
                switch (keyPressed->code) {
                case sf::Keyboard::Key::Escape: return; break;
                    // Player Movement
                case sf::Keyboard::Key::A: rocketGridPos.x--; break;
                case sf::Keyboard::Key::D: rocketGridPos.x++; break;
                case sf::Keyboard::Key::W: rocketGridPos.y--; break;
                case sf::Keyboard::Key::S: rocketGridPos.y++; break;

                    // Shooting Logic
                case sf::Keyboard::Key::Space: {
                    // Spawn bullet at the rocket's current grid position
                    GridObject newBullet;
                    newBullet.gridPos = rocketGridPos;
                    // Start its visual position exactly where the rocket currently is visually
                    newBullet.visualPos = rocketVisualPos;
                    bullets.push_back(newBullet);
                    break;
                }
                default: break;
                }

                // Clamp Rocket
                if (rocketGridPos.x < 0)            rocketGridPos.x = 0;
                if (rocketGridPos.x > COLS - 1)     rocketGridPos.x = COLS - 1;
                if (rocketGridPos.y < ROWS / 2)     rocketGridPos.y = ROWS / 2;
                if (rocketGridPos.y > ROWS - 1)     rocketGridPos.y = ROWS - 1;
            }
        }

        float dt = deltaClock.restart().asSeconds();

        // 2. Obstacle Spawning
        if (obstacleSpawnClock.getElapsedTime().asSeconds() >= SPAWN_INTERVAL) {
            GridObject newObstacle;
            newObstacle.gridPos = { std::rand() % COLS, 0 }; // Random column, top row
            newObstacle.visualPos = { static_cast<float>(newObstacle.gridPos.x * CELL_SIZE), -static_cast<float>(CELL_SIZE) }; // Start slightly off-screen
            obstacles.push_back(newObstacle);

            obstacleSpawnClock.restart();
        }

        // 3. Grid Logic Updates (Fixed Game Tick)
        if (gameTickClock.getElapsedTime().asSeconds() >= GAME_TICK_INTERVAL) {

            // Move Obstacles Downwards
            for (auto& obs : obstacles) {
                obs.gridPos.y++;
            }

            // Move Bullets Upwards
            for (auto& bullet : bullets) {
                bullet.gridPos.y--;
            }

            // Clean up out-of-bounds obstacles (passed the bottom)
            obstacles.erase(
                std::remove_if(obstacles.begin(), obstacles.end(), [&](const GridObject& o) { return o.gridPos.y >= ROWS; }),
                obstacles.end()
            );

            // Clean up out-of-bounds bullets (passed the top)
            bullets.erase(
                std::remove_if(bullets.begin(), bullets.end(), [&](const GridObject& b) { return b.gridPos.y < 0; }),
                bullets.end()
            );

            gameTickClock.restart();
        }

        // 4. Smooth Visual Interpolation (Lerp)

        // Smooth Rocket
        sf::Vector2f rocketTarget = { static_cast<float>(rocketGridPos.x * CELL_SIZE), static_cast<float>(rocketGridPos.y * CELL_SIZE) };
        rocketVisualPos.x += (rocketTarget.x - rocketVisualPos.x) * SLIDE_SPEED * dt;
        rocketVisualPos.y += (rocketTarget.y - rocketVisualPos.y) * SLIDE_SPEED * dt;
        rocket.setPosition(rocketVisualPos);

        // Smooth Obstacles
        for (auto& obs : obstacles) {
            sf::Vector2f target = { static_cast<float>(obs.gridPos.x * CELL_SIZE), static_cast<float>(obs.gridPos.y * CELL_SIZE) };
            obs.visualPos.x += (target.x - obs.visualPos.x) * SLIDE_SPEED * dt;
            obs.visualPos.y += (target.y - obs.visualPos.y) * SLIDE_SPEED * dt;
        }

        // Smooth Bullets
        for (auto& bullet : bullets) {
            // Adjust X target slightly to center the slim bullet in the middle of the grid cell
            float centerXOffset = (CELL_SIZE - bulletShape.getSize().x) / 2.0f;
            sf::Vector2f target = { static_cast<float>(bullet.gridPos.x * CELL_SIZE) + centerXOffset, static_cast<float>(bullet.gridPos.y * CELL_SIZE) };

            bullet.visualPos.x += (target.x - bullet.visualPos.x) * SLIDE_SPEED * dt;
            bullet.visualPos.y += (target.y - bullet.visualPos.y) * SLIDE_SPEED * dt;
        }

        // 5. Rendering
        window.clear(sf::Color(10, 10, 10));
        window.draw(spaceSprite);
        // Draw Player
        window.draw(rocket);

        // Draw Obstacles
        for (const auto& obs : obstacles) {
            obstacleShape.setPosition(obs.visualPos);
            window.draw(obstacleShape);
        }

        // Draw Bullets
        for (const auto& bullet : bullets) {
            bulletShape.setPosition(bullet.visualPos);
            window.draw(bulletShape);
        }

        window.display();
    }
}