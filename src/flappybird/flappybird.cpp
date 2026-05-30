#include <SFML/Graphics.hpp>

void runFlappyBird(sf::RenderWindow& window) {

    float cellW = (float)window.getSize().x / 12.f;
    float cellH = (float)window.getSize().y / 16.f;

    // bird
    sf::Texture birdTexture;
    if (!birdTexture.loadFromFile("assets/Popat.png")) return;
    sf::Sprite popat(birdTexture);
    float birdScaleX = (float)window.getSize().x / birdTexture.getSize().x * 0.05f;
    float birdScaleY = (float)window.getSize().y / birdTexture.getSize().y * 0.1f;
    popat.setScale({ birdScaleX, birdScaleY });
    popat.setPosition({ cellW * 2.f, cellH * 8.f });

    // background
    sf::Texture bgTexture;
    if (!bgTexture.loadFromFile("assets/Background.png")) return;
    sf::Sprite background(bgTexture);
    float scaleX = (float)window.getSize().x / bgTexture.getSize().x;
    float scaleY = (float)window.getSize().y / bgTexture.getSize().y;
    background.setScale({ scaleX, scaleY });

 

    while (window.isOpen()) {

       
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            
        }

        
        
        window.clear();
        window.draw(background);
		window.draw(popat);
        

        window.display();
    }
}