#include "rocketshooter/rocketshooter.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <fstream>

// Instantiating static shape fields
sf::RectangleShape RocketBullet::shape;
sf::RectangleShape RocketObstacle::shape;
sf::RectangleShape RocketCoin::shape;
sf::RectangleShape RocketShooterPlayer::shape;

// ==========================================
// --- RocketBullet Implementation ---
// ==========================================
RocketBullet::RocketBullet(sf::Vector2i startGridPos, sf::Vector2f startVisualPos, const sf::Texture& texture)
    : data{ startGridPos, startVisualPos }
{
    bulletSprite.emplace(texture);
    sf::Vector2u textureSize = texture.getSize();
    bulletSprite->setScale({ (0.2f * 32.0f) / textureSize.x, (0.6f * 32.0f) / textureSize.y });
}

void RocketBullet::moveUp() {
    data.gridPos.y--;
}

void RocketBullet::updateVisual(float slideSpeed, float dt, float cellSize) {
    float centerXOffset = (cellSize - shape.getSize().x) / 2.0f;
    sf::Vector2f target = { static_cast<float>(data.gridPos.x * cellSize) + centerXOffset, static_cast<float>(data.gridPos.y * cellSize) };

    data.visualPos.x += (target.x - data.visualPos.x) * slideSpeed * dt;
    data.visualPos.y += (target.y - data.visualPos.y) * slideSpeed * dt;

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

void RocketShooterPlayer::init(int cols, int rows, float cellSize, const sf::Texture& texture) {
    float targetSize = 3.25f * cellSize;
    shape.setSize({ targetSize, targetSize });
    shape.setFillColor(sf::Color::Red);

    gridPos = { (cols / 2) - 1, rows - 3 };
    visualPos = { static_cast<float>(gridPos.x * cellSize), static_cast<float>(gridPos.y * cellSize) };
    shape.setPosition(visualPos);

    playerSprite.emplace(texture);
    sf::Vector2u textureSize = texture.getSize();
    playerSprite->setScale({ targetSize / textureSize.x, targetSize / textureSize.y });
    playerSprite->setPosition(visualPos);
}

void RocketShooterPlayer::handleInput(sf::Keyboard::Key key) {
    switch (key) {
    case sf::Keyboard::Key::A: gridPos.x--; break;
    case sf::Keyboard::Key::D: gridPos.x++; break;
    case sf::Keyboard::Key::W: gridPos.y--; break;
    case sf::Keyboard::Key::S: gridPos.y++; break;
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
RocketShooterGame::RocketShooterGame(sf::RenderWindow& win)
    : window(win)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    cols = static_cast<int>(window.getSize().x / CELL_SIZE);
    rows = static_cast<int>(window.getSize().y / CELL_SIZE);

    if (!playerTexture.loadFromFile("assets/rocket/RedRocket.png")) std::cerr << "Failed player asset\n";
    if (!bulletTexture.loadFromFile("assets/rocket/bullet.png")) std::cerr << "Failed bullet asset\n";
    if (!obstacleTexture.loadFromFile("assets/rocket/asteroid.png")) std::cerr << "Failed asteroid asset\n";
    if (!bigObstacleTexture.loadFromFile("assets/rocket/bigasteroid.png")) std::cerr << "Failed big asteroid asset\n";
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
    RocketCoin::shape.setFillColor(sf::Color(255, 215, 0));

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

    const char* fontCandidates[] = {
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
    std::ifstream in(HIGH_SCORE_FILE, std::ios::in | std::ios::binary);
    if (!in.is_open()) {
        highScore = 0;
        return;
    }

    int savedScore = 0;
    if (in >> savedScore && savedScore >= 0) {
        highScore = savedScore;
    }
    else {
        highScore = 0;
    }
}

void RocketShooterGame::saveHighScore() {
    std::ofstream out(HIGH_SCORE_FILE, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to write " << HIGH_SCORE_FILE << "!\n";
        return;
    }
    out << highScore;
}

void RocketShooterGame::restartGame() {
    player.init(cols, rows, CELL_SIZE, playerTexture);
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

    obstacleSpawnClock.restart();
    gameTickClock.restart();
    deltaClock.restart();

    if (music.getStatus() != sf::Music::Status::Playing)
        music.play();
}

bool RocketShooterGame::playerCollidesWithObstacle(const RocketObstacle& obs) const {
    int playerMinX = player.gridPos.x;
    int playerMaxX = player.gridPos.x + 2; // Footprint spanning 3 cells
    int playerMinY = player.gridPos.y;
    int playerMaxY = player.gridPos.y + 2;

    int obsMinX = obs.data.gridPos.x;
    int obsMaxX = obs.data.gridPos.x + (obs.sizeInCells - 1);
    int obsMinY = obs.data.gridPos.y;
    int obsMaxY = obs.data.gridPos.y + (obs.sizeInCells - 1);

    return playerMinX <= obsMaxX && playerMaxX >= obsMinX &&
        playerMinY <= obsMaxY && playerMaxY >= obsMinY;
}

bool RocketShooterGame::bulletCollidesWithObstacle(const RocketBullet& b, const RocketObstacle& obs) const {
    int obsMinX = obs.data.gridPos.x;
    int obsMaxX = obs.data.gridPos.x + (obs.sizeInCells - 1);
    int obsMinY = obs.data.gridPos.y;
    int obsMaxY = obs.data.gridPos.y + (obs.sizeInCells - 1);

    return b.data.gridPos.x >= obsMinX && b.data.gridPos.x <= obsMaxX &&
        b.data.gridPos.y >= obsMinY && b.data.gridPos.y <= obsMaxY;
}

bool RocketShooterGame::bulletCollidesWithCoin(const RocketBullet& b, const RocketCoin& coin) const {
    return b.data.gridPos.x == coin.data.gridPos.x && b.data.gridPos.y == coin.data.gridPos.y;
}

void RocketShooterGame::checkCollisions() {
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
            player.init(cols, rows, CELL_SIZE, playerTexture);
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
                float playerWidth = 3.25f * CELL_SIZE;
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
            asteroidCounter++;
            bool spawnBig = (asteroidCounter % 9 == 4 || asteroidCounter % 9 == 8); // 2 spawns out of every 9
            const sf::Texture& texToUse = spawnBig ? bigObstacleTexture : obstacleTexture;
            obstacles.push_back(RocketObstacle(cols, CELL_SIZE, texToUse, spawnBig));
        }
    }
}

void RocketShooterGame::updateGridLogic() {
    if (gameTickClock.getElapsedTime().asSeconds() >= GAME_TICK_INTERVAL) {
        for (auto& obs : obstacles)   obs.moveDown();
        for (auto& bullet : bullets)  bullet.moveUp();
        for (auto& coin : fallingCoins) coin.moveDown();

        obstacles.erase(
            std::remove_if(obstacles.begin(), obstacles.end(), [&](const RocketObstacle& o) { return o.data.gridPos.y >= rows; }),
            obstacles.end()
        );

        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(), [&](const RocketBullet& b) { return b.data.gridPos.y < 0; }),
            bullets.end()
        );

        fallingCoins.erase(
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
            RocketObstacle::shape.setSize({ static_cast<float>(obs.sizeInCells * CELL_SIZE), static_cast<float>(obs.sizeInCells * CELL_SIZE) });
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

    if (fontLoaded) {
        sf::Text hudText(font, "", 34u);
        hudText.setFillColor(sf::Color::White);
        hudText.setPosition({ 15.f, 15.f });
        hudText.setString("Score: " + std::to_string(score) + "    High Score: " + std::to_string(highScore) + "    Coins: " + std::to_string(coins) + "    Lives: " + std::to_string(lives) + (isPaused ? "    [PAUSED]" : ""));
        window.draw(hudText);
    };

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

        if (!gameOver && !isPaused) {
            spawnObstacles();
            updateGridLogic();
            interpolateVisuals(dt);
        }
        render();
    }
}

void runRocketShooter(sf::RenderWindow& window) {
    RocketShooterGame game(window);
    game.run();
}