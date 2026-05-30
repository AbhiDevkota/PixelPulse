#include <SFML/Graphics.hpp>

void runFlappyBird(sf::RenderWindow& window) {


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
        
        window.display();
    }
}