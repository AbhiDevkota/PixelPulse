#include <iostream>
#include <SFML/Graphics.hpp>
#include<cstdlib>
using namespace std;
void RocketShooter(sf::RenderWindow& window){
cout << "Inital phase: RocketShooter!" << endl;
sf::RectangleShape rocket({40.f,70.f});
rocket.setFillColor(sf::Color:Green);
rocket.setPosition({220.f, 500.f}); // Put it somewhere visible
while (window.isOpen()) {
        while (auto e = window.pollEvent()) {
            if (e->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        window.clear(sf::Color::Black);
        window.draw(rocket);
        window.display();
    }
}
});
int main() {
    //small window
    sf::RenderWindow window(sf::VideoMode({ 480, 640 }), "RocketShooter");
    window.setFramerateLimit(60); //setting the performance frame rate
    RocketShooter(window);  //calling main rocketshooter function
    return 0;
}
