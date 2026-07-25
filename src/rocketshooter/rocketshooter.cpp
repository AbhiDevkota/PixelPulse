#include "rocketshooter/rocketshooter.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>

// Instantiating static shape fields
sf::RectangleShape RocketBullet::shape;
sf::RectangleShape RocketObstacle::shape;
sf::RectangleShape RocketCoin::shape;

// ==========================================
// --- RocketBullet Implementation ---
// ==========================================
RocketBullet::RocketBullet(sf::Vector2i startGridPos, sf::Vector2f startVisualPos, const sf::Texture& texture)
    : data{ startGridPos, startVisualPos }
{
    bulletSprite.emplace(texture);
    sf::Vector2u textureSize = texture.getSize();
    bulletSprite->setScale({ (0.2f * CELL_SIZE) / textureSize.x, (0.6f * CELL_SIZE) / textureSize.y });
    bulletSprite->setPosition(data.visualPos);
}

void RocketBullet::moveUp() {
    data.gridPos.y--;
}

void RocketBullet::updateVisual(float slideSpeed, float dt, float cellSize) {
    float targetY = static_cast<float>(data.gridPos.y * cellSize);
    data.visualPos.y += (targetY - data.visualPos.y) * slideSpeed * dt;

    if (bulletSprite.has_value()) {
        bulletSprite->setPosition(data.visualPos);
    }
}

// ==========================================
// --- RocketObstacle Implementation ---
// ==========================================
RocketObstacle::RocketObstacle(int totalCols, float cellSize, const sf::Texture& texture, bool big)
    : isBig(big)
{
    sizeInCells = isBig ? 3 : 2;
    scoreValue = isBig ? 3 : 1;

    data.gridPos = { std::rand() % (totalCols - (sizeInCells - 1)), 0 };
    data.visualPos = { static_cast<float>(data.gridPos.x * cellSize), -cellSize * sizeInCells };

    obstacleSprite.emplace(texture);
    sf::Vector2u textureSize = texture.getSize();
    float targetSize = static_cast<float>(sizeInCells) * cellSize;
    obstacleSprite->setScale({ targetSize / textureSize.x, targetSize / textureSize.y });
    obstacleSprite->setPosition(data.visualPos);
}

void RocketObstacle::moveDown() {
    data.gridPos.y++;
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
    data.gridPos.y++;
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
RocketShooterPlayer::RocketShooterPlayer() : gridPos(data.gridPos), visualPos(data.visualPos) {
    data.gridPos = { 0, 0 };
    data.visualPos = { 0.f, 0.f };
}

void RocketShooterPlayer::init(int cols, int rows, float cellSize, const sf::Texture& normTex, const sf::Texture& mTex, const sf::Texture& exp1, const sf::Texture& exp2) {
    normalTex = normTex;
    moveTex = mTex;
    explodeTex1 = exp1;
    explodeTex2 = exp2;

    isExploding = false;
    explosionFinished = false;
    explosionTimer = 0.0f;
    explosionFrame = 0;
    isMovingForward = false;

    float targetSize = 3.25f * cellSize;
    shape.setSize({ targetSize, targetSize });
    shape.setFillColor(sf::Color::Red);

    gridPos = { (cols / 2) - 1, rows - 3 };
    visualPos = { static_cast<float>(gridPos.x * cellSize), static_cast<float>(gridPos.y * cellSize) };
    shape.setPosition(visualPos);

    playerSprite.emplace(normalTex);
    applyTexture(normalTex);
}

void RocketShooterPlayer::applyTexture(const sf::Texture& tex, bool isExplosion) {
    if (!playerSprite.has_value()) return;

    playerSprite->setTexture(tex, true);
    float targetSize = 3.25f * CELL_SIZE;

    if (isExplosion) {
        playerSprite->setRotation(sf::degrees(-90.f));
        playerSprite->setOrigin({ 104.0f, 0.0f });
        float scaleFactor = targetSize / 104.0f;
        playerSprite->setScale({ scaleFactor, scaleFactor });
    }
    else {
        playerSprite->setRotation(sf::degrees(0.f));
        playerSprite->setOrigin({ 0.f, 0.f });
        sf::Vector2u texSize = tex.getSize();
        playerSprite->setScale({ targetSize / texSize.x, targetSize / texSize.y });
    }
    playerSprite->setPosition(visualPos);
}

void RocketShooterPlayer::triggerExplosion() {
    if (isExploding) return;
    isExploding = true;
    explosionFinished = false;
    explosionTimer = 0.0f;
    explosionFrame = 1;

    applyTexture(explodeTex1, true);
}

void RocketShooterPlayer::handleInput(sf::Keyboard::Key key) {
    if (isExploding) return;

    switch (key) {
    case sf::Keyboard::Key::A:
    case sf::Keyboard::Key::Left:
        gridPos.x--; break;
    case sf::Keyboard::Key::D:
    case sf::Keyboard::Key::Right:
        gridPos.x++; break;
    case sf::Keyboard::Key::W:
    case sf::Keyboard::Key::Up:
        gridPos.y--;
        isMovingForward = true;
        applyTexture(moveTex);
        break;
    case sf::Keyboard::Key::S:
    case sf::Keyboard::Key::Down:
        gridPos.y++; break;
    default: break;
    }
}

void RocketShooterPlayer::clampPosition(int cols, int rows) {
    if (gridPos.x < 0)         gridPos.x = 0;
    if (gridPos.x > cols - 3)  gridPos.x = cols - 3;
    if (gridPos.y < 0)         gridPos.y = 0;
    if (gridPos.y > rows - 3)  gridPos.y = rows - 3;
}

void RocketShooterPlayer::updateVisual(float slideSpeed, float dt, float cellSize) {
    if (isExploding) {
        explosionTimer += dt;

        if (explosionTimer >= 0.15f && explosionFrame == 1) {
            explosionFrame = 2;
            applyTexture(explodeTex2, true);
        }

        if (explosionTimer >= 0.35f) {
            explosionFinished = true;
        }
        return;
    }

    // Return to default normal texture when reaching target grid position
    sf::Vector2f target = { static_cast<float>(gridPos.x * cellSize), static_cast<float>(gridPos.y * cellSize) };

    if (isMovingForward && std::abs(target.y - visualPos.y) < 1.0f) {
        isMovingForward = false;
        applyTexture(normalTex);
    }

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
RocketShooterGame::RocketShooterGame(sf::RenderWindow& win, corezone::FileManager& filemanager)
    : window(win)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    cols = static_cast<int>(window.getSize().x / CELL_SIZE);
    rows = static_cast<int>(window.getSize().y / CELL_SIZE);

    // Owns high score / save-data persistence via the shared corezone file system,
    // namespaced under "ROCKETSHOOTER" so it doesn't collide with other games' saves.
    gameData = std::make_unique<corezone::GameDataManager>(filemanager, "ROCKETSHOOTER");

    if (!playerTexture.loadFromFile("assets/rocket/RedRocket.png")) std::cerr << "Failed player asset\n";
    if (!playerMoveTexture.loadFromFile("assets/rocket/move.png")) std::cerr << "Failed move asset\n";
    if (!explode1Texture.loadFromFile("assets/rocket/explode1.png")) std::cerr << "Failed explode1 asset\n";
    if (!explode2Texture.loadFromFile("assets/rocket/explode2.png")) std::cerr << "Failed explode2 asset\n";
    if (!bulletTexture.loadFromFile("assets/rocket/bullet2.png")) std::cerr << "Failed bullet2 asset\n";
    if (!obstacleTexture.loadFromFile("assets/rocket/asteroid.png")) std::cerr << "Failed asteroid asset\n";
    if (!bigObstacleTexture.loadFromFile("assets/rocket/bigasteroid.png")) std::cerr << "Failed big asteroid asset\n";
    if (!coinTexture.loadFromFile("assets/rocket/coinpic.png")) std::cerr << "Failed coin asset\n";

    if (!coinSoundBuffer.loadFromFile("audios/rocket/coineffect.ogg")) {
        std::cerr << "Failed to load coin sound!\n";
    }
    else {
        coinSound.emplace(coinSoundBuffer);
        coinSound->setVolume(50.f);
    }

    if (!explosionSoundBuffer.loadFromFile("audios/rocket/explosionsound.ogg")) {
        std::cerr << "Failed to load explosion sound!\n";
    }
    else {
        explosionSound.emplace(explosionSoundBuffer);
        explosionSound->setVolume(60.f);
    }

    player.init(cols, rows, CELL_SIZE, playerTexture, playerMoveTexture, explode1Texture, explode2Texture);

    RocketObstacle::shape.setSize({ static_cast<float>(2.0 * CELL_SIZE), static_cast<float>(2.0 * CELL_SIZE) });
    RocketObstacle::shape.setFillColor(sf::Color::Blue);

    RocketBullet::shape.setSize({ static_cast<float>(CELL_SIZE) * 0.2f, static_cast<float>(CELL_SIZE) * 0.6f });
    RocketBullet::shape.setFillColor(sf::Color::Yellow);

    RocketCoin::shape.setSize({ static_cast<float>(CELL_SIZE), static_cast<float>(CELL_SIZE) });
    RocketCoin::shape.setFillColor(sf::Color(255, 215, 0));

    if (!spaceTexture.loadFromFile("assets/rocket/starsrocket.png")) {
        std::cerr << "Failed to load space texture!\n";
    }
    else {
        spaceTexture.setRepeated(true);
        spaceSprite.emplace(spaceTexture);
        spaceSprite->setTextureRect(sf::IntRect({ 0, 0 }, static_cast<sf::Vector2i>(window.getSize())));
    }

    if (!music.openFromFile("audios/rocket/spacemusic.ogg")) {
        std::cerr << "Failed to load music!\n";
    }
    else {
        music.setLooping(true);
        music.setVolume(24.f);
        music.play();
    }

    if (font.openFromFile("fonts/regular.ttf")) {
        fontLoaded = true;
        setupGameOverText();
    }

    gameOverFallbackBar.setSize({ 400.f, 60.f });
    gameOverFallbackBar.setOrigin({ 200.f, 30.f });
    gameOverFallbackBar.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f - 50.f });

    restartFallbackBar.setSize({ 400.f, 40.f });
    restartFallbackBar.setOrigin({ 200.f, 20.f });
    restartFallbackBar.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f + 30.f });

    loadHighScore();
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

void RocketShooterGame::loadHighScore() {
    highScore = 0;
    if (gameData) {
        // Reads from config/ROCKETSHOOTER_highscore_data via GameDataManager.
        // If the file doesn't exist yet or fails to parse, getHighScore leaves
        // highScore untouched (or fails), so we keep the safe default of 0.
        gameData->getHighScore(highScore);
    }
}

void RocketShooterGame::saveHighScore() {
    if (gameData) {
        // Writes to config/ROCKETSHOOTER_highscore_data via GameDataManager.
        if (!gameData->saveHighScore(highScore)) {
            std::cerr << "Failed to save high score for ROCKETSHOOTER\n";
        }
    }
}

void RocketShooterGame::syncHighScoreFromDisk() {
    if (!gameData) return;

    int diskScore = 0;
    if (!gameData->getHighScore(diskScore)) return;

    // Pick up manual edits to the .sav file in either direction. Since any
    // in-session increase is saved to disk immediately (see checkCollisions),
    // the file is always the source of truth by the time this runs.
    if (diskScore != highScore) {
        highScore = diskScore;
    }
}

void RocketShooterGame::restartGame() {
    player.init(cols, rows, CELL_SIZE, playerTexture, playerMoveTexture, explode1Texture, explode2Texture);
    bullets.clear();
    obstacles.clear();
    fallingCoins.clear();
    obstaclesSpawnedCount = 0;
    asteroidCounter = 0;
    gameOver = false;
    isPaused = false;
    lives = 3;
    score = 0;
    coins = 0;

    obstacleTickInterval = GAME_TICK_INTERVAL;
    scoreMilestone = 0;

    obstacleSpawnClock.restart();
    gameTickClock.restart();
    obstacleTickClock.restart();
    deltaClock.restart();
    highScoreSyncClock.restart();

    if (music.getStatus() != sf::Music::Status::Playing) music.play();
}

bool RocketShooterGame::playerCollidesWithObstacle(const RocketObstacle& obs) const {
    int playerMinX = player.gridPos.x;
    int playerMaxX = player.gridPos.x + 2;
    int playerMinY = player.gridPos.y;
    int playerMaxY = player.gridPos.y + 2;

    int obsMinX = obs.data.gridPos.x;
    int obsMaxX = obs.data.gridPos.x + (obs.sizeInCells - 1);
    int obsMinY = obs.data.gridPos.y;
    int obsMaxY = obs.data.gridPos.y + (obs.sizeInCells - 1);

    return playerMinX <= obsMaxX && playerMaxX >= obsMinX && playerMinY <= obsMaxY && playerMaxY >= obsMinY;
}

bool RocketShooterGame::playerCollidesWithCoin(const RocketCoin& coin) const {
    int playerMinX = player.gridPos.x;
    int playerMaxX = player.gridPos.x + 2;
    int playerMinY = player.gridPos.y;
    int playerMaxY = player.gridPos.y + 2;

    return coin.data.gridPos.x >= playerMinX && coin.data.gridPos.x <= playerMaxX &&
        coin.data.gridPos.y >= playerMinY && coin.data.gridPos.y <= playerMaxY;
}

bool RocketShooterGame::bulletCollidesWithObstacle(const RocketBullet& b, const RocketObstacle& obs) const {
    int obsMinX = obs.data.gridPos.x;
    int obsMaxX = obs.data.gridPos.x + (obs.sizeInCells - 1);
    int obsMinY = obs.data.gridPos.y;
    int obsMaxY = obs.data.gridPos.y + (obs.sizeInCells - 1);

    return b.data.gridPos.x >= obsMinX && b.data.gridPos.x <= obsMaxX &&
        b.data.gridPos.y >= obsMinY && b.data.gridPos.y <= obsMaxY;
}

void RocketShooterGame::updateDifficulty() {
    int newMilestone = (score / SCORE_MILESTONE_STEP) * SCORE_MILESTONE_STEP;
    if (newMilestone > scoreMilestone) {
        scoreMilestone = newMilestone;
        obstacleTickInterval = std::max(MIN_OBSTACLE_TICK_INTERVAL, obstacleTickInterval * OBSTACLE_SPEED_MULTIPLIER);
    }
}

void RocketShooterGame::checkCollisions() {
    if (player.isExploding) return;

    std::vector<bool> obstacleHit(obstacles.size(), false);
    std::vector<bool> bulletHit(bullets.size(), false);

    for (size_t bi = 0; bi < bullets.size(); bi++) {
        for (size_t oi = 0; oi < obstacles.size(); oi++) {
            if (!obstacleHit[oi] && bulletCollidesWithObstacle(bullets[bi], obstacles[oi])) {
                obstacleHit[oi] = true;
                bulletHit[bi] = true;
            }
        }
    }

    for (int i = static_cast<int>(bullets.size()) - 1; i >= 0; i--) {
        if (bulletHit[i]) bullets.erase(bullets.begin() + i);
    }

    for (int i = static_cast<int>(obstacles.size()) - 1; i >= 0; i--) {
        if (obstacleHit[i]) {
            score += obstacles[i].scoreValue;
            obstacles.erase(obstacles.begin() + i);
            if (score > highScore) {
                highScore = score;
                saveHighScore();
            }
            updateDifficulty();
        }
    }

    for (const auto& obs : obstacles) {
        if (playerCollidesWithObstacle(obs)) {
            player.triggerExplosion();
            if (explosionSound.has_value()) {
                explosionSound->play();
            }
            break;
        }
    }
}

void RocketShooterGame::checkCoinPickups() {
    if (player.isExploding) return;

    for (int i = static_cast<int>(fallingCoins.size()) - 1; i >= 0; i--) {
        if (playerCollidesWithCoin(fallingCoins[i])) {
            fallingCoins.erase(fallingCoins.begin() + i);
            coins++;
            if (coinSound.has_value()) {
                coinSound->play();
            }
        }
    }
}

void RocketShooterGame::handlePlayerHit() {
    lives--;
    obstacles.clear();
    bullets.clear();

    if (lives <= 0) {
        gameOver = true;
        music.stop();
    }
    else {
        player.init(cols, rows, CELL_SIZE, playerTexture, playerMoveTexture, explode1Texture, explode2Texture);
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

            if (isPaused || player.isExploding) continue;

            player.handleInput(keyPressed->code);
            player.clampPosition(cols, rows);

            if (keyPressed->code == sf::Keyboard::Key::Space) {
                float playerWidth = 3.25f * CELL_SIZE;
                float bulletWidth = 0.2f * CELL_SIZE;

                sf::Vector2f centeredVisualPos = player.visualPos;
                centeredVisualPos.x += (playerWidth / 2.0f) - (bulletWidth / 2.0f);

                sf::Vector2i bulletGridPos = player.gridPos;
                bulletGridPos.x += 1;

                bullets.push_back(RocketBullet(bulletGridPos, centeredVisualPos, bulletTexture));
            }
        }
    }
}

void RocketShooterGame::spawnObstacles() {
    if (player.isExploding) return;

    if (obstacleSpawnClock.getElapsedTime().asSeconds() >= SPAWN_INTERVAL) {
        obstacleSpawnClock.restart();
        obstaclesSpawnedCount++;

        if (obstaclesSpawnedCount >= OBSTACLES_PER_COIN) {
            obstaclesSpawnedCount = 0;
            int spawnCol = std::rand() % cols;
            fallingCoins.push_back(RocketCoin({ spawnCol, 0 }, CELL_SIZE, coinTexture));
        }
        else {
            asteroidCounter++;
            bool spawnBig = (asteroidCounter % 9 == 4 || asteroidCounter % 9 == 8);
            const sf::Texture& texToUse = spawnBig ? bigObstacleTexture : obstacleTexture;
            obstacles.push_back(RocketObstacle(cols, CELL_SIZE, texToUse, spawnBig));
        }
    }
}

void RocketShooterGame::updateGridLogic() {
    // Bullets and coins keep a fixed tick rate.
    if (gameTickClock.getElapsedTime().asSeconds() >= GAME_TICK_INTERVAL) {
        if (!player.isExploding) {
            for (auto& bullet : bullets)  bullet.moveUp();
            for (auto& coin : fallingCoins) coin.moveDown();

            bullets.erase(
                std::remove_if(bullets.begin(), bullets.end(), [&](const RocketBullet& b) { return b.data.gridPos.y < 0; }),
                bullets.end()
            );

            fallingCoins.erase(
                std::remove_if(fallingCoins.begin(), fallingCoins.end(), [&](const RocketCoin& c) { return c.data.gridPos.y >= rows; }),
                fallingCoins.end()
            );

            checkCoinPickups();
        }
        gameTickClock.restart();
    }

    // Obstacles tick on their own clock, which speeds up (shrinks) as the score climbs.
    if (obstacleTickClock.getElapsedTime().asSeconds() >= obstacleTickInterval) {
        if (!player.isExploding) {
            for (auto& obs : obstacles) obs.moveDown();

            obstacles.erase(
                std::remove_if(obstacles.begin(), obstacles.end(), [&](const RocketObstacle& o) { return o.data.gridPos.y >= rows; }),
                obstacles.end()
            );

            checkCollisions();
        }
        obstacleTickClock.restart();
    }
}

void RocketShooterGame::interpolateVisuals(float dt) {
    player.updateVisual(SLIDE_SPEED, dt, CELL_SIZE);

    if (player.isExploding && player.explosionFinished) {
        handlePlayerHit();
    }

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
        else window.draw(RocketObstacle::shape);
    }

    for (const auto& coin : fallingCoins) {
        if (coin.coinSprite.has_value()) window.draw(*coin.coinSprite);
        else window.draw(RocketCoin::shape);
    }

    for (const auto& bullet : bullets) {
        if (bullet.bulletSprite.has_value()) window.draw(*bullet.bulletSprite);
        else window.draw(RocketBullet::shape);
    }

    if (fontLoaded) {
        sf::Text hudText(font, "", 24u);
        hudText.setFillColor(sf::Color::White);
        hudText.setPosition({ 15.f, 15.f });
        hudText.setString("Score: " + std::to_string(score) + "    High Score: " + std::to_string(highScore) + "    Coins: " + std::to_string(coins) + "    Lives: " + std::to_string(lives) + (isPaused ? "    [PAUSED]" : ""));
        window.draw(hudText);
    }

    if (gameOver) {
        sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)));
        overlay.setFillColor(sf::Color(0, 0, 0, 160));
        window.draw(overlay);

        if (fontLoaded && gameOverText) {
            window.draw(*gameOverText);

            sf::Text finalScoreText(font, "Final Score: " + std::to_string(score) + "  (High Score: " + std::to_string(highScore) + " | Coins: " + std::to_string(coins) + ")", 24u);
            finalScoreText.setFillColor(sf::Color::White);
            sf::FloatRect sBounds = finalScoreText.getLocalBounds();
            finalScoreText.setOrigin({ sBounds.size.x / 2.f, sBounds.size.y / 2.f });
            finalScoreText.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f });
            window.draw(finalScoreText);
        }
        else window.draw(gameOverFallbackBar);

        if (fontLoaded && restartText) window.draw(*restartText);
        else window.draw(restartFallbackBar);
    }

    window.display();
}

void RocketShooterGame::run() {
    while (window.isOpen() && !exitToMenu) {
        float dt = deltaClock.restart().asSeconds();
        handleEvents();

        if (highScoreSyncClock.getElapsedTime().asSeconds() >= HIGH_SCORE_SYNC_INTERVAL) {
            syncHighScoreFromDisk();
            highScoreSyncClock.restart();
        }

        if (!gameOver && !isPaused) {
            spawnObstacles();
            updateGridLogic();
            interpolateVisuals(dt);
        }
        render();
    }
}

void runRocketShooter(sf::RenderWindow& window, corezone::FileManager& filemanager) {
    RocketShooterGame game(window, filemanager);
    game.run();
}