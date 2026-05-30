#include <SFML/Graphics.hpp>

void runFlappyBird(sf::RenderWindow& window) {

    float cellW = (float)window.getSize().x / 12.f;
    float cellH = (float)window.getSize().y / 16.f;

    //bird
    sf::Texture birdTexture;
    if (!birdTexture.loadFromFile("assets/Popat.png")) return;
    sf::Sprite popat(birdTexture);
    float birdScaleX = (float)window.getSize().x / birdTexture.getSize().x * 0.05f;
    float birdScaleY = (float)window.getSize().y / birdTexture.getSize().y * 0.1f;
    popat.setScale({ birdScaleX, birdScaleY });
    popat.setPosition({ cellW * 2.f, cellH * 8.f });

    //background
    sf::Texture bgTexture;
    if (!bgTexture.loadFromFile("assets/Background.png")) return;
    sf::Sprite background(bgTexture);
    float scaleX = (float)window.getSize().x / bgTexture.getSize().x;
    float scaleY = (float)window.getSize().y / bgTexture.getSize().y;
    background.setScale({ scaleX, scaleY });

 
    //pipes
    sf::Texture upTexture;
    if (!upTexture.loadFromFile("assets/Up.png")) return;
    sf::Sprite pipeUp(upTexture);
    float pipeScaleX = (float)window.getSize().x / upTexture.getSize().x * 0.15f;
    float pipeScaleY = (float)window.getSize().y / upTexture.getSize().y * 0.4f;
    pipeUp.setScale({ pipeScaleX, pipeScaleY });

    sf::Texture downTexture;
    if (!downTexture.loadFromFile("assets/Down.png")) return;
    sf::Sprite pipeDown(downTexture);
    pipeDown.setScale({ pipeScaleX, pipeScaleY });

    float pipeX = cellW * 12.f;
    float gapY = cellH * 8.f;
    float gapSize = cellH * 4.f;

    pipeDown.setPosition({ pipeX, gapY - gapSize / 2.f - pipeDown.getGlobalBounds().size.y });
    pipeUp.setPosition({ pipeX, gapY + gapSize / 2.f });

    while (window.isOpen()) {

       
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            
        }

        
        
        window.clear();
        window.draw(background);
        window.draw(pipeDown);
        window.draw(pipeUp);
		window.draw(popat);
		
        

        window.display();
    }
}