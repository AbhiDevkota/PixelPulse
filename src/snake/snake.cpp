#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "snake/snake.h"
#include "snake/snakeassets.h"
#include "files.h"
#include "common/highscore.h"

Snake::Snake(sf::Vector2i startPos, sf::Vector2i startDir, int cols, int rows)
    : cols(cols), rows(rows) {
    reset(startPos, startDir);
    respawnFood();
}

void Snake::reset(sf::Vector2i startPos, sf::Vector2i startDir) {
    body.clear();
    direction = startDir;
    nextDirection = startDir;
    growPending = false;
    for (int i = 0; i < 3; ++i)
        body.push_back({ startPos.x - startDir.x * i, startPos.y - startDir.y * i });
}

void Snake::setDirection(sf::Vector2i dir) {
    if (dir.x == -direction.x && dir.y == -direction.y)
        return;
    nextDirection = dir;
}

void Snake::move() {
    direction = nextDirection;
    sf::Vector2i newHead = body.front() + direction;
    body.push_front(newHead);

    if (growPending)
        growPending = false; // skip popping the tail this tick, so the snake grows by one cell
    else
        body.pop_back();
}

void Snake::grow() {
     growPending = true;
}

bool Snake::checkSelfCollision() const {
    sf::Vector2i head = body.front();
    for (size_t i = 1; i < body.size(); ++i)
        if (body[i] == head) return true;
    return false;
}

bool Snake::checkWallCollision() const {
    sf::Vector2i head = body.front();
    return head.x < 0 || head.y < 0 || head.x >= cols || head.y >= rows;
}

bool Snake::checkFoodCollision() const {
    return body.front() == foodPos;
}

void Snake::respawnFood() {
    foodPos = { std::rand() % cols, std::rand() % rows };
}

sf::Vector2i Snake::getFoodPosition() const { return foodPos; }
const std::deque<sf::Vector2i>& Snake::getBody() const { return body; }
sf::Vector2i Snake::getHead() const { return body.front(); }
sf::Vector2i Snake::getDirection() const { return direction; }

float directionToRotation(sf::Vector2i dir) {
    if (dir == sf::Vector2i{ 0, 1 })  return 0.f;
    if (dir == sf::Vector2i{ 0, -1 }) return 180.f;
    if (dir == sf::Vector2i{ 1, 0 })  return 270.f;
    if (dir == sf::Vector2i{ -1, 0 }) return 90.f;
    return 0.f;
}

float cornerRotation(sf::Vector2i a, sf::Vector2i b) {
    sf::Vector2i top{ 0,-1 }, right{ 1,0 }, bottom{ 0,1 }, left{ -1,0 };
    if ((a == top && b == right) || (a == right && b == top)) return 0.f;
    if ((a == right && b == bottom) || (a == bottom && b == right)) return 90.f;
    if ((a == bottom && b == left) || (a == left && b == bottom)) return 180.f;
    if ((a == left && b == top) || (a == top && b == left)) return 270.f;
    return 0.f;
}

void runSnake(sf::RenderWindow& window, corezone::FileManager& filemanager) {
    corezone::GameDataManager gameData(filemanager, "SNAKE");

    int lives = 3;
    HighScore highScoreObj(gameData);

    bool isPaused = false;
    bool gameOver = false;
    const int CELL_SIZE = 32;
    auto winSize = window.getSize();
    const int TARGET = static_cast<int>(std::min(winSize.x, winSize.y) * 0.78f);
    const int PLAY_HEIGHT = (TARGET / CELL_SIZE) * CELL_SIZE;
    const int PLAY_WIDTH = PLAY_HEIGHT;
    const int COLS = PLAY_HEIGHT / CELL_SIZE;
    const int ROWS = PLAY_WIDTH / CELL_SIZE;
    const int OFFSETX = (winSize.x - PLAY_WIDTH) / 2;
    const int OFFSETY = (winSize.y - PLAY_HEIGHT) / 2;

    int score = 0;

    SnakeLayout layout;
    layout.cellSize = CELL_SIZE;
    layout.cols = COLS;
    layout.rows = ROWS;
    layout.playWidth = PLAY_WIDTH;
    layout.playHeight = PLAY_HEIGHT;
    layout.offsetX = OFFSETX;
    layout.offsetY = OFFSETY;

    SnakeAssets assets;
    if (!assets.loadAll(layout, winSize)) {
        std::cerr << "runSnake: assets failed to load\n";
        return;
    }
    assets.getMusic().play();

    sf::Text scoreText(assets.getFont());
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition({ (float)OFFSETX, (float)(OFFSETY - 2 * CELL_SIZE) });

    sf::Clock clock;
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    float moveInterval = 0.2f;

    Snake snake({ 5, 5 }, { 1, 0 }, COLS, ROWS);

    while (window.isOpen()) {
        while (auto e = window.pollEvent()) {
            if (e->is<sf::Event::Closed>())
                window.close();
            if (e->is<sf::Event::KeyPressed>()) {
                auto key = e->getIf<sf::Event::KeyPressed>()->code;
                if (key == sf::Keyboard::Key::Escape)
                    return;
                if (key == sf::Keyboard::Key::R && gameOver) {
                    snake.reset({ 5, 5 }, { 1, 0 });
                    lives = 3;
                    score = 0;
                    gameOver = false;
                    snake.respawnFood();
                }
                if (key == sf::Keyboard::Key::Space || key == sf::Keyboard::Key::P)
                    isPaused = !isPaused;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            snake.setDirection({ 0, -1 });
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            snake.setDirection({ -1, 0 });
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            snake.setDirection({ 0, 1 });
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            snake.setDirection({ 1, 0 });

        if (!gameOver && !isPaused && clock.getElapsedTime().asSeconds() >= moveInterval) {
            snake.move();
            clock.restart();

            if (snake.checkSelfCollision() || snake.checkWallCollision()) {
                lives--;
                if (lives <= 0) {
                    gameOver = true;
                }
                else {
                    snake.reset({ 5, 5 }, { 1, 0 });
                    clock.restart();
                }
            }

            if (snake.checkFoodCollision()) {
                snake.respawnFood();
                assets.randomizeFoodTexture();
                assets.getEatSound().play();
                snake.grow();
                score++;
                highScoreObj.set(score);
            }
        }

        assets.getFoodSprite().setPosition({
            (float)(OFFSETX + snake.getFoodPosition().x * CELL_SIZE),
            (float)(OFFSETY + snake.getFoodPosition().y * CELL_SIZE)
            });

        window.clear(sf::Color(10, 10, 10));
        assets.drawBackground(window);

        if (gameOver) {
            scoreText.setCharacterSize(32);
            scoreText.setString(
                "\tGame Over!  \n\nScore: " + std::to_string(score) +
                "\nHigh Score: " + std::to_string(highScoreObj.get()) +
                "\n\nPress R to Restart"
            
            );
            sf::FloatRect textBounds = scoreText.getLocalBounds();
            scoreText.setPosition({
                (float)(window.getSize().x / 2) - textBounds.size.x / 2,
                (float)(window.getSize().y / 2) - textBounds.size.y / 2
                });
            window.draw(scoreText);
            window.display();
            continue;
        }

        scoreText.setCharacterSize(24);
        scoreText.setString("Score: " + std::to_string(score) + "  High Score: " + std::to_string(highScoreObj.get()) + "  Lives:  " + std::to_string(lives));
        scoreText.setPosition({ (float)OFFSETX, (float)(OFFSETY - 2 * CELL_SIZE) });
        window.draw(scoreText);
        window.draw(assets.getFoodSprite());

        const auto& body = snake.getBody();
        for (size_t i = 0; i < body.size(); i++) {
            sf::Vector2i pos = body[i];
            float centerX = OFFSETX + pos.x * CELL_SIZE + CELL_SIZE / 2.f;
            float centerY = OFFSETY + pos.y * CELL_SIZE + CELL_SIZE / 2.f;

            if (i == 0) {
                auto& head = assets.getHeadSprite();
                head.setRotation(sf::degrees(directionToRotation(snake.getDirection())));
                head.setPosition({ centerX, centerY });
                window.draw(head);
            }
            else if (i == body.size() - 1) {
                // tail
                sf::Vector2i segDir = body[i - 1] - pos;
                if (segDir.x == 0 && segDir.y == 0)
                    segDir = snake.getDirection(); // avoid glitch when a duplicated segment overlaps the tail after eating
                auto& tail = assets.getTailSprite();
                tail.setRotation(sf::degrees(directionToRotation(segDir)));
                tail.setPosition({ centerX, centerY });
                window.draw(tail);
            }
            else {
                sf::Vector2i dirToHead = body[i - 1] - pos;
                sf::Vector2i dirToTail = body[i + 1] - pos;
                bool isStraight = (dirToHead.x + dirToTail.x == 0) && (dirToHead.y + dirToTail.y == 0);

                if (isStraight) {
                    auto& bodySeg = assets.getBodySprite();
                    bodySeg.setRotation(sf::degrees(directionToRotation(dirToHead)));
                    bodySeg.setPosition({ centerX, centerY });
                    window.draw(bodySeg);
                }
                else {
                    auto& corner = assets.getCornerSprite();
                    corner.setRotation(sf::degrees(cornerRotation(dirToHead, dirToTail)));
                    corner.setPosition({ centerX, centerY });
                    window.draw(corner);
                }
            }
        }

        window.display();
    }
}