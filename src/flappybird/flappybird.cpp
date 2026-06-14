#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>

void runFlappyBird(sf::RenderWindow& window) {

    std::srand((unsigned int)std::time(nullptr));

    //background music
    sf::Music music;
    if (!music.openFromFile("audios/Neverfelt.mp3")) return;
    music.setLooping(true);  
    music.play();         
    

    float cellW = (float)window.getSize().x / 12.f;
    float cellH = (float)window.getSize().y / 16.f;

    //bird
    sf::Texture birdTexture;
    if (!birdTexture.loadFromFile("assets/Flappy/Popat.png")) return;
    sf::Sprite popat(birdTexture);
    float birdScaleX = (float)window.getSize().x / birdTexture.getSize().x * 0.05f;
    float birdScaleY = (float)window.getSize().y / birdTexture.getSize().y * 0.1f;
    popat.setScale({ birdScaleX, birdScaleY });
    popat.setPosition({ cellW * 2.f, cellH * 8.f });           //popat position

    //background
    sf::Texture bgTexture;
    if (!bgTexture.loadFromFile("assets/Flappy/Sky.png")) return;
    sf::Sprite background(bgTexture);
    float scaleX = (float)window.getSize().x / bgTexture.getSize().x;
    float scaleY = (float)window.getSize().y / bgTexture.getSize().y;
	background.setScale({ scaleX, scaleY });                //background size

 
    //pipes
    sf::Texture upTexture;
    if (!upTexture.loadFromFile("assets/Flappy/Up.png")) return;
    sf::Sprite pipeUp(upTexture);
    float pipeScaleX = (float)window.getSize().x / upTexture.getSize().x * 0.15f;
    float pipeScaleY = (float)window.getSize().y / upTexture.getSize().y * 0.4f;
    pipeUp.setScale({ pipeScaleX, pipeScaleY });        //bottom pipe size

    sf::Texture downTexture;
    if (!downTexture.loadFromFile("assets/Flappy/Down.png")) return;
    sf::Sprite pipeDown(downTexture);
    pipeDown.setScale({ pipeScaleX, pipeScaleY });      //top pipe size

    

    // anchor pipeDown by its top-left -> top stays fixed at y=0 when scaled
    pipeDown.setOrigin({ 0.f, 0.f });

    // anchor pipeUp by its bottom-left -> bottom stays fixed at screen bottom when scaled
    pipeUp.setOrigin({ 0.f, (float)upTexture.getSize().y });

    float pipeX = cellW * 12.f;            //pipe starts from the right edge of the screen
    float gapY = cellH * 8.f;              //gap center
    float gapSize = cellH * 4.f;           //gap size

    // scale first pipe using the same formula as later spawns
    float pipeDownHeight = gapY - gapSize / 2.f;
    pipeDown.setScale({ pipeScaleX, pipeDownHeight / downTexture.getSize().y });

    float pipeUpHeight = (float)window.getSize().y - (gapY + gapSize / 2.f);
    pipeUp.setScale({ pipeScaleX, pipeUpHeight / upTexture.getSize().y });

    pipeDown.setPosition({ pipeX, 0.f });
    pipeUp.setPosition({ pipeX, (float)window.getSize().y });
    //physics
    float vy = 0.f;                      //bird velocity
    float gravity = 1000.f;              //gravity

    sf::Clock clock;

    while (window.isOpen()) {

		float dt = clock.restart().asSeconds();     //get time between frames as sf::time and convert it to seconds as float
       
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                return;
            }
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
				vy = -600.f;                   //jump velocity
			}
            
        }

		//update
		vy += gravity * dt;                 //vertical velocity affected by gravity
        popat.move({ 0.f, vy * dt });      //bird position

        //popat boundary

        float birdH = popat.getGlobalBounds().size.y;
        float birdY = popat.getPosition().y;

        if (birdY < 0.f) {
            popat.setPosition({ popat.getPosition().x, 0.f });     //prevent bird from going above the screen
            vy = 0.f;                      //stop vertical movement
        }

        if (birdH + birdY > window.getSize().y) {
            popat.setPosition({ popat.getPosition().x, window.getSize().y - birdH });
            vy = 0.f;
        }
       
        if (birdH + birdY > window.getSize().y) {
            popat.setPosition({ popat.getPosition().x, window.getSize().y - birdH });
            vy = 0.f;
        }

        pipeX -= cellW * 3.f * dt;
        pipeDown.setPosition({ pipeX, 0.f });
        pipeUp.setPosition({ pipeX, (float)window.getSize().y });

        if (pipeX + pipeUp.getGlobalBounds().size.x < 0.f) {
            pipeX = cellW * 12.f;

            // pick a random gap centre (gap SIZE stays constant)
            float minGapY = cellH * 4.f;
            float maxGapY = cellH * 12.f;
            gapY = minGapY + (float)(std::rand() % (int)(maxGapY - minGapY));

            // pipeDown: from top of screen (y=0) to gap start
            float pipeDownHeight = gapY - gapSize / 2.f;
            pipeDown.setScale({ pipeScaleX, pipeDownHeight / downTexture.getSize().y });

            // pipeUp: from gap end to bottom of screen
            float pipeUpHeight = (float)window.getSize().y - (gapY + gapSize / 2.f);
            pipeUp.setScale({ pipeScaleX, pipeUpHeight / upTexture.getSize().y });

            pipeDown.setPosition({ pipeX, 0.f });
            pipeUp.setPosition({ pipeX, (float)window.getSize().y });
        }



        // collision detection
        if (popat.getGlobalBounds().findIntersection(pipeUp.getGlobalBounds()) ||
            popat.getGlobalBounds().findIntersection(pipeDown.getGlobalBounds())) {
            return; // game over - exits back to homescreen
        }

        //draw

        window.clear();
        window.draw(background);
        window.draw(pipeDown);
        window.draw(pipeUp);
		window.draw(popat);
		
        

        window.display();
    }
}