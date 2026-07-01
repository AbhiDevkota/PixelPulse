#include "flappybird.h"
#include "background.h"
#include "bird.h"
#include "pipepair.h"
#include "gameaudio.h"
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


    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // Input
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
                return;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
                bird.flap();
                audio.playJump();
            }
        }

        // Update
        bird.update(dt, window);
        pipes.update(dt, window, cellW, cellH);
        background.update(dt, window);
            
        // Add score when bird passes pipe
        score += pipes.getScorePoint(bird.getBounds().position.x);

        // Update high score and persist it (same pattern as Snake)
        if (score > highScore) {
            highScore = score;
            gameData.saveHighScore(highScore);
        }


        // Update score text
        scoreText.setString("Score: " + std::to_string(score));
        highScoreText.setString("High Score: " + std::to_string(highScore));

        // Collision
        if (pipes.collides(bird.getBounds())) {
            background.pickRandom(window);
            bird.reset(cellW, cellH);
            pipes.reset(window, cellW, cellH);
                
            // Reset score after collision
            score = 0;
            scoreText.setString("Score: 0");

            sf::sleep(sf::milliseconds(500));
        }

        // Draw
        window.clear();
        background.draw(window);
        pipes.draw(window);
        bird.draw(window);

        // Draw score at top-left
        window.draw(scoreText);
        window.draw(highScoreText);

        window.display();
    }
}