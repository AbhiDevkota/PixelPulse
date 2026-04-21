#ifndef HOMESCREEN_H
#define HOMESCREEN_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include <memory>

namespace corezone {

/**
 * @brief Configuration class for home screen settings
 */
class Config {
public:
    static const float BG_COLOR;
    static const float GRID_STRENGTH;
    static const int INTRO_DURATION_MS;
    static const std::string INTRO_INITIAL_TEXT;
};

/**
 * @brief Manages the menu items (game selection)
 */
class Menu {
private:
    std::vector<std::string> items_;
    int selectedIndex_ = 0;
    int itemHeight_ = 0;
    float yPos_ = 0.0f;

public:
    Menu();
    void addItem(const std::string& name);
    void selectNext();
    void selectPrev();
    const std::string& getSelectedName() const;
    sf::FloatRect getBounds() const;

    // Getters for private members
    int getSelectedIndex() const { return selectedIndex_; }
    int getItemHeight() const { return itemHeight_; }
    float getYPos() const { return yPos_; }
    const std::vector<std::string>& getItems() const { return items_; }

    // Setters
    void setSelectedIndex(int index) { selectedIndex_ = index; }
    void setItemHeight(int height) { itemHeight_ = height; }
    void setYPos(float y) { yPos_ = y; }
};

/**
 * @brief Handles animation and visual effects
 */
class Effects {
private:
    bool introVisible_ = true;
    bool flashActive_ = false;

public:
    void showIntro();
    void hideIntro();
    void triggerFlash();
    bool isIntroVisible() const;
};

/**
 * @brief Main home screen controller
 *
 * Manages the complete home screen including:
 * - Background with grid pattern and scanlines
 * - Title text with flicker animation
 * - Menu navigation
 * - Hint text display
 * - Intro animation sequence
 * - Sound management
 */
class HomeScreen {
private:
    // Resources
    sf::Font font_;
    std::string fontPath_;

    std::string homeMusicPath_;
    std::string selectSoundPath_;

    // Sound managers
    std::unique_ptr<sf::Music> homeMusic_;
    std::unique_ptr<sf::SoundBuffer> selectSoundBuffer_;
    std::unique_ptr<sf::Sound> selectSound_;

    // Components
    std::unique_ptr<Menu> menu_;
    std::unique_ptr<Effects> effects_;

    // Visual elements
    sf::RenderWindow& window_;

    // Layout state
    int itemHeight_ = 0;
    float yPos_ = 0.0f;

    // Layout constants
    const float GRID_SPACING = 28.0f;
    const float TITLE_FONT_SIZE = 38.0f;
    const float TITLE_LETTER_SPACING = 0.5f;
    const float HINT_FONT_SIZE = 0.9f;
    const float LEFT_PADDING = 32.0f;
    const float RIGHT_PADDING = 32.0f;

    // State
    bool introFinished_ = false;
    bool isRunning_ = true;
    bool flashActive_ = false;
    bool introVisible_ = true;

    // Animation timing
    float introOpacity_ = 1.0f;
    float introAlphaRate_ = 0.1f;
    float animationTimer_ = 0.0f;
    float blinkTimer_ = 0.0f;

public:
    /**
     * @brief Constructs a HomeScreen instance
     * @param window The SFML window to render to
     * @param fontPath Path to the font file
     * @param homeMusicPath Path to the home screen music
     * @param selectSoundPath Path to the selection sound effect
     */
    HomeScreen(sf::RenderWindow& window, const std::string& fontPath,
               const std::string& homeMusicPath, const std::string& selectSoundPath);

    /**
     * @brief Destroys the HomeScreen instance
     */
    ~HomeScreen() = default;

    /**
     * @brief Main update loop - processes events and updates state
     * @param deltaTime Time in seconds since last update
     */
    void update(float deltaTime);

    /**
     * @brief Draws all home screen elements to the window
     */
    void draw();

    /**
     * @brief Initializes the home screen resources
     */
    void initialize();

    /**
     * @brief Sets up all menu items
     */
    void setupMenu();

    /**
     * @brief Handles keyboard input for navigation
     * @param event The SFML event to process
     */
    void handleInput(const sf::Event& event);

    /**
     * @brief Handles window resize events
     * @param event The SFML resize event
     */
    void handleResize(const sf::Event::Resized& event);

    /**
     * @brief Shown when a game is selected
     */
    void onGameSelected();

    /**
     * @brief Cleanup resources on destruction
     */
    void cleanup();

private:
    /**
     * @brief Renders the grid background
     */
    void renderBackground();

    /**
     * @brief Renders the intro overlay
     */
    void renderIntro();

    /**
     * @brief Renders the flash effect
     */
    void renderFlash();

    /**
     * @brief Renders the title text
     */
    void renderTitle();

    /**
     * @brief Renders all menu items
     */
    void renderMenu();

    /**
     * @brief Renders the hint text
     */
    void renderHint();

    /**
     * @brief Loads and plays the home screen music
     */
    void playMusic();

    /**
     * @brief Plays the selection sound effect
     */
    void playSelectSound();

    /**
     * @brief Animates the intro text
     */
    void animateIntro(float deltaTime);
};

} // namespace corezone

#endif // HOMESCREEN_H
