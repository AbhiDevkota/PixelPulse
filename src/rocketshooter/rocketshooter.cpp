#include "rocketshooter/rocketshooter.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>

// Instantiating static shape fields allocated across memory scopes
sf::RectangleShape RocketBullet::shape;
sf::RectangleShape RocketObstacle::shape;
sf::RectangleShape RocketCoin::shape;

// ==========================================
// --- RocketBullet Implementation ---
// ==========================================
RocketBullet::RocketBullet(sf::Vector2i startGridPos, sf::Vector2f startVisualPos, const sf::Texture& texture)
    : data{ startGridPos, startVisualPos }
{
    bulletSprite.emplace(texture); // Initialize optional sprite
    sf::Vector2u textureSize = texture.getSize();
    bulletSprite->setScale({ (0.2f * 32.0f) / textureSize.x, (0.6f * 32.0f) / textureSize.y });
}

void RocketBullet::moveUp() {
    data.gridPos.y--; // Move up one grid cell
}

void RocketBullet::updateVisual(float slideSpeed, float dt, float cellSize) {
    float centerXOffset = (cellSize - shape.getSize().x) / 2.0f; // Center bullet horizontally
    sf::Vector2f target = { static_cast<float>(data.gridPos.x * cellSize) + centerXOffset, static_cast<float>(data.gridPos.y * cellSize) };

    data.visualPos.x += (target.x - data.visualPos.x) * slideSpeed * dt; // Linear interpolation for X
    data.visualPos.y += (target.y - data.visualPos.y) * slideSpeed * dt; // Linear interpolation for Y

    if (bulletSprite.has_value()) {
        bulletSprite->setPosition(data.visualPos);
    }
}

// ==========================================
// --- RocketObstacle Implementation ---
// ==========================================
RocketObstacle::RocketObstacle(int totalCols, float cellSize, const sf::Texture& texture) {
    data.gridPos = { std::rand() % totalCols, 0 }; // Random starting column
    data.visualPos = { static_cast<float>(data.gridPos.x * cellSize), -cellSize };

    obstacleSprite.emplace(texture);
    sf::Vector2u textureSize = texture.getSize();
    float targetSize = 2.0f * cellSize;
    obstacleSprite->setScale({ targetSize / textureSize.x, targetSize / textureSize.y });
    obstacleSprite->setPosition(data.visualPos);
}

void RocketObstacle::moveDown() {
    data.gridPos.y++; // Move down one grid cell
}

void RocketObstacle::updateVisual(float slideSpeed, float dt, float cellSize) {
    sf::Vector2f target = { static_cast<float>(data.gridPos.x * cellSize), static_cast<float>(data.gridPos.y * cellSize) };
    data.visualPos.x += (target.x - data.visualPos.x) * slideSpeed * dt;
    data.visualPos.y += (target.y - data.visualPos.y) * slideSpeed * dt;

    if (obstacleSprite.has_value()) {
        obstacleSprite->setPosition(data.visualPos);
    }
}

// ==========================================
// --- RocketCoin Implementation ---
// ==========================================
RocketCoin::RocketCoin(sf::Vector2i startGridPos, float cellSize, const sf::Texture& texture) {
    data.gridPos = startGridPos;
    data.visualPos = { static_cast<float>(data.gridPos.x * cellSize), -cellSize };

    coinSprite.emplace(texture);
    sf::Vector2u textureSize = texture.getSize();
    float targetSize = 1.0f * cellSize;
    coinSprite->setScale({ targetSize / textureSize.x, targetSize / textureSize.y });
    coinSprite->setPosition(data.visualPos);
}

void RocketCoin::moveDown() {
    data.gridPos.y++; // Move down one grid cell
}

void RocketCoin::updateVisual(float slideSpeed, float dt, float cellSize) {
    sf::Vector2f target = { static_cast<float>(data.gridPos.x * cellSize), static_cast<float>(data.gridPos.y * cellSize) };
    data.visualPos.x += (target.x - data.visualPos.x) * slideSpeed * dt;
    data.visualPos.y += (target.y - data.visualPos.y) * slideSpeed * dt;

    if (coinSprite.has_value()) {
        coinSprite->setPosition(data.visualPos);
    }
}

// ==========================================
// --- RocketShooterPlayer Implementation ---
// ==========================================
RocketShooterPlayer::RocketShooterPlayer() : gridPos(0, 0), visualPos(0.f, 0.f) {}

void RocketShooterPlayer::init(int cols, int rows, float cellSize, const sf::Texture& texture) {
    shape.setSize({ static_cast<float>(2 * cellSize), static_cast<float>(2 * cellSize) });
    shape.setFillColor(sf::Color::Red);

    gridPos = { (cols / 2) - 1, rows - 2 }; // Center player horizontally near bottom
    visualPos = { static_cast<float>(gridPos.x * cellSize), static_cast<float>(gridPos.y * cellSize) };
    shape.setPosition(visualPos);

    playerSprite.emplace(texture);
    sf::Vector2u textureSize = texture.getSize();
    float targetSize = 2.0f * cellSize;
    playerSprite->setScale({ targetSize / textureSize.x, targetSize / textureSize.y });
    playerSprite->setPosition(visualPos);
}

void RocketShooterPlayer::handleInput(sf::Keyboard::Key key) {
    switch (key) {
    case sf::Keyboard::Key::A: gridPos.x--; break; // Left
    case sf::Keyboard::Key::D: gridPos.x++; break; // Right
    case sf::Keyboard::Key::W: gridPos.y--; break; // Up
    case sf::Keyboard::Key::S: gridPos.y++; break; // Down
    default: break;
    }
}

void RocketShooterPlayer::clampPosition(int cols, int rows) {
    // Player occupies a 2x2 grid footprint, so it must stay within
    // [0, cols-2] horizontally and [0, rows-2] vertically to remain
    // fully on screen. This now spans the entire play field instead
    // of being restricted to the bottom rows.
    if (gridPos.x < 0)        gridPos.x = 0;        // Left boundary
    if (gridPos.x > cols - 2) gridPos.x = cols - 2;  // Right boundary (2 cells wide)
    if (gridPos.y < 0)        gridPos.y = 0;         // Top boundary
    if (gridPos.y > rows - 2) gridPos.y = rows - 2;  // Bottom boundary (2 cells tall)
}

void RocketShooterPlayer::updateVisual(float slideSpeed, float dt, float cellSize) {
    sf::Vector2f target = { static_cast<float>(gridPos.x * cellSize), static_cast<float>(gridPos.y * cellSize) };
    visualPos.x += (target.x - visualPos.x) * slideSpeed * dt;
    visualPos.y += (target.y - visualPos.y) * slideSpeed * dt;
    shape.setPosition(visualPos);

    if (playerSprite.has_value()) {
        playerSprite->setPosition(visualPos);
    }
}

// ==========================================
// --- Game Engine Implementation ---
// ==========================================
RocketShooterGame::RocketShooterGame(sf::RenderWindow& win, corezone::FileManager& fileManager)
    : window(win), gameData(fileManager, "ROCKETSHOOTER")
{
    std::srand(static_cast<unsigned>(std::time(nullptr))); // Seed RNG
    cols = static_cast<int>(window.getSize().x / CELL_SIZE);
    rows = static_cast<int>(window.getSize().y / CELL_SIZE);

    if (!playerTexture.loadFromFile("assets/rocket/RedRocket.png")) std::cerr << "Failed player asset\n";
    if (!bulletTexture.loadFromFile("assets/rocket/bullet.png")) std::cerr << "Failed bullet asset\n";
    if (!obstacleTexture.loadFromFile("assets/rocket/asteroid.png")) std::cerr << "Failed asteroid asset\n";
    if (!coinTexture.loadFromFile("assets/rocket/coinpic.png")) std::cerr << "Failed coin asset\n";

    if (!coinSoundBuffer.loadFromFile("audios/rocket/coineffect.ogg")) {
        std::cerr << "Failed to load audios/rocket/coineffect.ogg!\n";
    }
    else {
        coinSound.emplace(coinSoundBuffer);
        coinSound->setVolume(50.f);
    }

    player.init(cols, rows, CELL_SIZE, playerTexture);

    RocketObstacle::shape.setSize({ static_cast<float>(2.0 * CELL_SIZE), static_cast<float>(2.0 * CELL_SIZE) });
    RocketObstacle::shape.setFillColor(sf::Color::Blue);

    RocketBullet::shape.setSize({ static_cast<float>(CELL_SIZE) * 0.2f, static_cast<float>(CELL_SIZE) * 0.6f });
    RocketBullet::shape.setFillColor(sf::Color::Yellow);

    RocketCoin::shape.setSize({ static_cast<float>(CELL_SIZE), static_cast<float>(CELL_SIZE) });
    RocketCoin::shape.setFillColor(sf::Color(255, 215, 0)); // Gold fallback

    if (!spaceTexture.loadFromFile("assets/rocket/starsrocket.png")) {
        std::cerr << "Failed to load background texture!\n";
    }
    else {
        spaceTexture.setRepeated(true);
        spaceSprite.emplace(spaceTexture);
        spaceSprite->setTextureRect(sf::IntRect({ 0, 0 }, { (int)window.getSize().x, (int)window.getSize().y }));
    }

    if (!music.openFromFile("audios/rocket/spacemusic.ogg")) {
        std::cerr << "Failed to load background music!\n";
    }
    else {
        music.setLooping(true);
        music.setVolume(24.f);
        music.play();
    }

    const char* fontCandidates[] = { // Multi-platform font paths targeting regular.ttf variants exclusively
        "D:\\projects\\PixelPulse\\fonts\\regular.ttf",
        "assets/fonts/regular.ttf"
    };

    for (const char* path : fontCandidates) {
        if (font.openFromFile(path)) {
            fontLoaded = true;
            break;
        }
    }

    if (fontLoaded) {
        setupGameOverText();
    }

    gameOverFallbackBar.setSize({ 400.f, 60.f });
    gameOverFallbackBar.setOrigin({ 200.f, 30.f }); // Center origin
    gameOverFallbackBar.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f - 50.f });

    restartFallbackBar.setSize({ 400.f, 40.f });
    restartFallbackBar.setOrigin({ 200.f, 20.f }); // Center origin
    restartFallbackBar.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f + 30.f });

    highScore = std::make_unique<HighScore>(gameData);
}

void RocketShooterGame::setupGameOverText() {
    gameOverText.emplace(font, "GAME OVER", 64u);
    gameOverText->setFillColor(sf::Color::Red);
    gameOverText->setStyle(sf::Text::Bold);
    sf::FloatRect bounds = gameOverText->getLocalBounds();
    gameOverText->setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
    gameOverText->setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f - 50.f });

    restartText.emplace(font, "Press R to Restart  |  ESC to Menu", 28u);
    restartText->setFillColor(sf::Color::White);
    sf::FloatRect rBounds = restartText->getLocalBounds();
    restartText->setOrigin({ rBounds.size.x / 2.f, rBounds.size.y / 2.f });
    restartText->setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f + 30.f });
}

void RocketShooterGame::restartGame() {
    player.init(cols, rows, CELL_SIZE, playerTexture);
    bullets.clear();      // Wipe entities
    obstacles.clear();    // Wipe entities
    fallingCoins.clear(); // Wipe entities
    obstaclesSpawnedCount = 0;
    gameOver = false;
    isPaused = false;
    lives = 3;
    score = 0;
    coins = 0;

    obstacleSpawnClock.restart();
    gameTickClock.restart();
    deltaClock.restart();

    if (music.getStatus() != sf::Music::Status::Playing)
        music.play();
}

bool RocketShooterGame::playerCollidesWithObstacle(const RocketObstacle& obs) const {
    // Both the player and the obstacle now occupy a 2x2 footprint of grid
    // cells (asteroid sprite is 2 * CELL_SIZE). A single-cell equality
    // check missed most real overlaps, so this does a proper axis-aligned
    // bounding box (AABB) test between the two 2x2 footprints. Using <= / >=
    // means even edges just touching count as a collision.
    int playerMinX = player.gridPos.x;
    int playerMaxX = player.gridPos.x + 1;
    int playerMinY = player.gridPos.y;
    int playerMaxY = player.gridPos.y + 1;

    int obsMinX = obs.data.gridPos.x;
    int obsMaxX = obs.data.gridPos.x + 1;
    int obsMinY = obs.data.gridPos.y;
    int obsMaxY = obs.data.gridPos.y + 1;

    return playerMinX <= obsMaxX && playerMaxX >= obsMinX &&
        playerMinY <= obsMaxY && playerMaxY >= obsMinY;
}

bool RocketShooterGame::bulletCollidesWithObstacle(const RocketBullet& b, const RocketObstacle& obs) const {
    // The obstacle occupies a 2x2 footprint of grid cells (asteroid sprite
    // is 2 * CELL_SIZE), so a bullet should register a hit if its cell
    // falls anywhere within that footprint, not just an exact match with
    // the obstacle's single reference cell.
    int obsMinX = obs.data.gridPos.x;
    int obsMaxX = obs.data.gridPos.x + 1;
    int obsMinY = obs.data.gridPos.y;
    int obsMaxY = obs.data.gridPos.y + 1;

    return b.data.gridPos.x >= obsMinX && b.data.gridPos.x <= obsMaxX &&
        b.data.gridPos.y >= obsMinY && b.data.gridPos.y <= obsMaxY;
}

bool RocketShooterGame::bulletCollidesWithCoin(const RocketBullet& b, const RocketCoin& coin) const {
    return b.data.gridPos.x == coin.data.gridPos.x && b.data.gridPos.y == coin.data.gridPos.y; // Perfect cell match
}

void RocketShooterGame::checkCollisions() {
    std::vector<bool> obstacleHit(obstacles.size(), false);
    std::vector<bool> bulletHit(bullets.size(), false);

    for (size_t bi = 0; bi < bullets.size(); bi++) { // Matrix flag evaluation
        for (size_t oi = 0; oi < obstacles.size(); oi++) {
            if (!obstacleHit[oi] && bulletCollidesWithObstacle(bullets[bi], obstacles[oi])) {
                obstacleHit[oi] = true;
                bulletHit[bi] = true;
            }
        }
    }

    for (int i = static_cast<int>(bullets.size()) - 1; i >= 0; i--) { // Reverse cleanup loop
        if (bulletHit[i]) bullets.erase(bullets.begin() + i);
    }

    for (int i = static_cast<int>(obstacles.size()) - 1; i >= 0; i--) { // Reverse cleanup loop
        if (obstacleHit[i]) {
            obstacles.erase(obstacles.begin() + i);
            score++;
            highScore->set(score);
        }
    }

    bool playerHit = false;
    for (const auto& obs : obstacles) {
        if (playerCollidesWithObstacle(obs)) {
            playerHit = true;
            break;
        }
    }

    if (playerHit) {
        lives--;
        obstacles.clear();
        bullets.clear();
        if (lives <= 0) {
            gameOver = true;
            music.stop();
        }
        else {
            player.init(cols, rows, CELL_SIZE, playerTexture); // Respawn player
        }
    }
}

void RocketShooterGame::checkCoinPickups() {
    std::vector<bool> coinHit(fallingCoins.size(), false);
    std::vector<bool> bulletHit(bullets.size(), false);

    for (size_t bi = 0; bi < bullets.size(); bi++) {
        for (size_t ci = 0; ci < fallingCoins.size(); ci++) {
            if (!coinHit[ci] && bulletCollidesWithCoin(bullets[bi], fallingCoins[ci])) {
                coinHit[ci] = true;
                bulletHit[bi] = true;
            }
        }
    }

    for (int i = static_cast<int>(bullets.size()) - 1; i >= 0; i--) {
        if (bulletHit[i]) bullets.erase(bullets.begin() + i);
    }

    for (int i = static_cast<int>(fallingCoins.size()) - 1; i >= 0; i--) {
        if (coinHit[i]) {
            fallingCoins.erase(fallingCoins.begin() + i);
            coins++;
            if (coinSound.has_value()) {
                coinSound->play();
            }
        }
    }
}

void RocketShooterGame::handleEvents() {
    while (auto e = window.pollEvent()) {
        if (e->is<sf::Event::Closed>()) {
            window.close();
        }

        if (auto keyPressed = e->getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                exitToMenu = true;
                return;
            }

            if (gameOver) {
                if (keyPressed->code == sf::Keyboard::Key::R) restartGame();
                continue;
            }

            if (keyPressed->code == sf::Keyboard::Key::P) {
                isPaused = !isPaused;
                continue;
            }

            if (isPaused) continue;

            player.handleInput(keyPressed->code);
            player.clampPosition(cols, rows);

            if (keyPressed->code == sf::Keyboard::Key::Space) {
                float playerWidth = 2.0f * CELL_SIZE;
                float bulletWidth = 0.2f * CELL_SIZE;

                sf::Vector2f centeredVisualPos = player.visualPos;
                centeredVisualPos.x += (playerWidth / 2.0f) - (bulletWidth / 2.0f);

                bullets.push_back(RocketBullet(player.gridPos, centeredVisualPos, bulletTexture));
            }
        }
    }
}

void RocketShooterGame::spawnObstacles() {
    if (obstacleSpawnClock.getElapsedTime().asSeconds() >= SPAWN_INTERVAL) {
        obstacleSpawnClock.restart();
        obstaclesSpawnedCount++;

        if (obstaclesSpawnedCount >= OBSTACLES_PER_COIN) {
            obstaclesSpawnedCount = 0;
            int spawnCol = std::rand() % cols;
            fallingCoins.push_back(RocketCoin({ spawnCol, 0 }, CELL_SIZE, coinTexture));
        }
        else {
            obstacles.push_back(RocketObstacle(cols, CELL_SIZE, obstacleTexture));
        }
    }
}

void RocketShooterGame::updateGridLogic() {
    if (gameTickClock.getElapsedTime().asSeconds() >= GAME_TICK_INTERVAL) {
        for (auto& obs : obstacles)   obs.moveDown();
        for (auto& bullet : bullets)  bullet.moveUp();
        for (auto& coin : fallingCoins) coin.moveDown();

        obstacles.erase( // Erase out-of-bounds obstacles
            std::remove_if(obstacles.begin(), obstacles.end(), [&](const RocketObstacle& o) { return o.data.gridPos.y >= rows; }),
            obstacles.end()
        );

        bullets.erase( // Erase out-of-bounds bullets
            std::remove_if(bullets.begin(), bullets.end(), [&](const RocketBullet& b) { return b.data.gridPos.y < 0; }),
            bullets.end()
        );

        fallingCoins.erase( // Erase out-of-bounds coins
            std::remove_if(fallingCoins.begin(), fallingCoins.end(), [&](const RocketCoin& c) { return c.data.gridPos.y >= rows; }),
            fallingCoins.end()
        );

        checkCoinPickups();
        checkCollisions();
        gameTickClock.restart();
    }
}

void RocketShooterGame::interpolateVisuals(float dt) {
    player.updateVisual(SLIDE_SPEED, dt, CELL_SIZE);
    for (auto& obs : obstacles)  obs.updateVisual(SLIDE_SPEED, dt, CELL_SIZE);
    for (auto& bullet : bullets) bullet.updateVisual(SLIDE_SPEED, dt, CELL_SIZE);
    for (auto& coin : fallingCoins) coin.updateVisual(SLIDE_SPEED, dt, CELL_SIZE);
}

void RocketShooterGame::render() {
    window.clear(sf::Color(10, 10, 10));

    if (spaceSprite.has_value()) window.draw(*spaceSprite);

    if (player.playerSprite.has_value()) window.draw(*player.playerSprite);
    else window.draw(player.shape);

    for (const auto& obs : obstacles) {
        if (obs.obstacleSprite.has_value()) window.draw(*obs.obstacleSprite);
        else {
            RocketObstacle::shape.setPosition(obs.data.visualPos);
            window.draw(RocketObstacle::shape);
        }
    }

    for (const auto& coin : fallingCoins) {
        if (coin.coinSprite.has_value()) window.draw(*coin.coinSprite);
        else {
            RocketCoin::shape.setPosition(coin.data.visualPos);
            window.draw(RocketCoin::shape);
        }
    }

    for (const auto& bullet : bullets) {
        if (bullet.bulletSprite.has_value()) window.draw(*bullet.bulletSprite);
        else {
            RocketBullet::shape.setPosition(bullet.data.visualPos);
            window.draw(RocketBullet::shape);
        }
    }

    if (fontLoaded) { // Only rendering live parameters HUD layout if regular.ttf validated
        sf::Text hudText(font, "", 34u);
        hudText.setFillColor(sf::Color::White);
        hudText.setPosition({ 15.f, 15.f });
        hudText.setString("Score: " + std::to_string(score) + "    High Score: " + std::to_string(highScore->get()) + "    Coins: " + std::to_string(coins) + "    Lives: " + std::to_string(lives) + (isPaused ? "    [PAUSED]" : ""));
        window.draw(hudText);
    };

    if (gameOver) {
        sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)));
        overlay.setFillColor(sf::Color(0, 0, 0, 160)); // Translucent backdrop matrix
        window.draw(overlay);

        if (fontLoaded && gameOverText) { // Enforces local regular.ttf execution over text rendering
            window.draw(*gameOverText);

            sf::Text finalScoreText(font, "Final Score: " + std::to_string(score) + "  (High Score: " + std::to_string(highScore->get()) + " | Coins: " + std::to_string(coins) + ")", 24u);
            finalScoreText.setFillColor(sf::Color::White);
            sf::FloatRect sBounds = finalScoreText.getLocalBounds();
            finalScoreText.setOrigin({ sBounds.size.x / 2.f, sBounds.size.y / 2.f });
            finalScoreText.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f });
            window.draw(finalScoreText);
        }
        else window.draw(gameOverFallbackBar);

        if (fontLoaded && restartText) window.draw(*restartText); // Enforces absolute path match condition
        else window.draw(restartFallbackBar);
    }

    window.display();
}

void RocketShooterGame::run() {
    while (window.isOpen() && !exitToMenu) {
        float dt = deltaClock.restart().asSeconds(); // Calculate frame delta execution speed
        handleEvents();

        if (!gameOver && !isPaused) {
            spawnObstacles();
            updateGridLogic();
            interpolateVisuals(dt);
        }
        render();
    }
}

void runRocketShooter(sf::RenderWindow& window, corezone::FileManager& fileManager) {
    RocketShooterGame game(window, fileManager);
    game.run();
}