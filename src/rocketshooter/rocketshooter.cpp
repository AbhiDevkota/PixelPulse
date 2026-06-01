#include <iostream>
#include <SFML/Graphics.hpp>
#include <cstdlib>

using namespace std;

void RocketShooter(sf::RenderWindow& window) {
    cout << "Initial phase: RocketShooter!" << endl;
    // Create the rocket green rectangle
    sf::RectangleShape rocket({ 40.f, 70.f });
    rocket.setFillColor(sf::Color::Green); // Fixed: Changed ':' to '::'
    rocket.setPosition({ 220.f, 500.f });
    // Game loop
    while (window.isOpen()) {
        
        while (auto e = window.pollEvent()) {
			if(e->is < sf::Event::Closed>()) {
				window.close();
			}
        }
        // Rendering
        window.clear(sf::Color::Black);
        window.draw(rocket);
        window.display();
    }
}