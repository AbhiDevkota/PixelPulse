#ifndef HOMESCREEN_H
#define HOMESCREEN_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

namespace corezone {

    class Menu {
    private:
        std::vector<std::string> items_;
        int selectedIndex_ = 0;

    public:
        Menu();
        void selectNext();
        void selectPrev();
        const std::string& getSelected() const;
        int getSelectedIndex() const { return selectedIndex_; }
        const std::vector<std::string>& getItems() const { return items_; }
    };

    class HomeScreen {
    private:
        sf::RenderWindow& window_;
        sf::Font font_;
        bool fontLoaded_ = false;
        sf::Font iconFont_;

        Menu menu_;

        // Clocks
        sf::Clock introClock_;
        sf::Clock flickerClock_;
        sf::Clock blinkClock_;
        sf::Clock loadingClock_;
        sf::Clock beamClock_;           // ← vertical moving scanline

        // Boot state
        enum class State { Boot, Menu };
        State state_ = State::Boot;

        bool introVisible_ = true;
        std::string introText_ = "Developed by GROUP X";
        bool flashActive_ = false;
        bool loadingMode_ = false;
        std::string loadingName_;
        int dotCount_ = 0;

        float introOpacity_ = 255.0f;

    public:
        HomeScreen(sf::RenderWindow& window, const std::string& fontPath);
        void initialize();
        void update(float deltaTime);
        void handleInput(const sf::Event& event);
        void draw();

    private:
        void renderBackground();
        void renderIntro();
        void renderFlash();
        void renderTitle();
        void renderMenu();
        void renderHint();
        void renderControls();
        void renderCredit();
        void startLoading();
    };

} // namespace corezone

#endif