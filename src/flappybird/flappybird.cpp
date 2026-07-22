#include "flappy/flappybird.h"
#include "flappy/background.h"
#include "flappy/bird.h"
#include "flappy/pipepair.h"
#include "flappy/gameaudio.h"
#include "Files.h"
#include <cstdlib>
#include <ctime>

void runFlappyBird(sf::RenderWindow& window, corezone::FileManager& filemanager) {
    std::srand((unsigned int)std::time(nullptr));

    // Persistent high-score storage (same pattern as Snake)
    corezone::GameDataManager gameData(filemanager, "FLAPPYBIRD");

    // Load game audio
    GameAudio audio;
    if (!audio.load()) return;

    float cellW = (float)window.getSize().x / 12.f;
    float cellH = (float)window.getSize().y / 16.f;

    // Create game objects
    Background background(window);
    Bird bird(window, cellW, cellH);
    if (!bird.isLoaded()) return;

    PipePair pipes(window, cellW, cellH);
    if (!pipes.isLoaded()) return;

    // Score values
    int score = 0;
    int highScore = 0;
    gameData.getHighScore(highScore);   // load saved high score on start
    float bannerTimer = 0.f;            // counts down while "New High Score!" is shown
    bool newRecordSet = false;         // true only for the point that first breaks the old record

    // Load font from fonts folder
    sf::Font font;
    font.openFromFile("fonts/regular.ttf");

    // Score text at top-left
    sf::Text scoreText(font);
    scoreText.setCharacterSize(40);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition({ 20.f, 15.f });
    scoreText.setString("Score: 0");

    // High score text below score
    sf::Text highScoreText(font);
    highScoreText.setCharacterSize(28);
    highScoreText.setFillColor(sf::Color::White);
    highScoreText.setPosition({ 20.f, 60.f });
    highScoreText.setString("High Score: " + std::to_string(highScore));

    sf::Text gameOverText(font);
    gameOverText.setCharacterSize(42);
    gameOverText.setFillColor(sf::Color::White);

    bool gameOver = false;
    bool paused = false;
    bool pWasPressed = false;

    sf::Clock clock;

    while (window.isOpen()) {                           // ← outer game loop

        float dt = clock.restart().asSeconds();

        while (auto event = window.pollEvent()) {       // ← inner event loop

            if (event->is<sf::Event::Closed>())
                window.close();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
                return;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P) && !gameOver && !pWasPressed) {
                paused = !paused;
                pWasPressed = true;
            }

            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P)) {
                pWasPressed = false;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R) && gameOver) {
                background.pickRandom(window);
                bird.reset(cellW, cellH);
                pipes.reset(window, cellW, cellH);
                score = 0;
                scoreText.setString("Score: 0");
                paused = false;
                gameOver = false;
                newRecordSet = false;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && !gameOver && !paused) {
                bird.flap();
                audio.playJump();
            }
        }

        // freeze all game logic when game over
        if (!gameOver && !paused) {
            bird.update(dt, window);

            // end the game if the bird hits the top or bottom of the screen
            if (bird.sprite.getPosition().y <= 0.f ||
                bird.sprite.getPosition().y + bird.sprite.getGlobalBounds().size.y >= (float)window.getSize().y)
                gameOver = true;
            pipes.update(dt, window, cellW, cellH);
            background.update(dt, window);

            // add 1 if bird just passed a pipe, 0 otherwise
            score += pipes.getScorePoint(bird.getBounds().position.x);

            // update and save high score immediately when beaten
            if (score > highScore) {
                if (!newRecordSet) {
                    bannerTimer = 1.f;
                    newRecordSet = true;
                }
                highScore = score;
                gameData.saveHighScore(highScore);
            }
            bannerTimer -= dt;

            // refresh HUD text every frame
            scoreText.setString("Score: " + std::to_string(score));

            // briefly show "New High Score!" in place of the usual high score text
            sf::FloatRect b = highScoreText.getLocalBounds();
            if (bannerTimer > 0.f) {
                highScoreText.setCharacterSize(48);
                highScoreText.setFillColor(sf::Color::Yellow);
                highScoreText.setOutlineColor(sf::Color::Black);
                highScoreText.setOutlineThickness(3.f);
                highScoreText.setString("New High Score!");
                b = highScoreText.getLocalBounds();
                highScoreText.setPosition({ window.getSize().x / 2.f - b.size.x / 2.f, window.getSize().y / 2.f });
            }
            else {
                highScoreText.setCharacterSize(28);
                highScoreText.setFillColor(sf::Color::White);
                highScoreText.setOutlineThickness(0.f);
                highScoreText.setString("High Score: " + std::to_string(highScore));
                highScoreText.setPosition({ 20.f, 60.f });
            }

            // collision with any pipe triggers game over
            if (pipes.collides(bird.getBounds()))
                gameOver = true;
        }

        // draw background, pipes, bird every frame
        window.clear();
        background.draw(window);
        pipes.draw(window);
        bird.draw(window);

        // show HUD only while playing
        if (!gameOver) {
            window.draw(scoreText);
            window.draw(highScoreText);
        }


        // draw game over overlay on top when game over
        if (gameOver) {
            gameOverText.setString(
                "Game Over!  Score: " + std::to_string(score) +
                "\nHigh Score: " + std::to_string(highScore) +
                "\n\nPress R to Restart"
            );

            // center the text on screen
            sf::FloatRect bounds = gameOverText.getLocalBounds();
            gameOverText.setPosition({
                window.getSize().x / 2.f - bounds.size.x / 2.f - bounds.position.x,
                window.getSize().y / 2.f - bounds.size.y / 2.f - bounds.position.y
                });
            window.draw(gameOverText);
        }

        window.display();
    }
}