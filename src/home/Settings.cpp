#include "Settings.h"
#include <cmath>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <windows.h>
#include <shlobj.h>

namespace corezone {

    VolumeBar::VolumeBar(const sf::Font& font, const std::string& label)
        : label_(label), labelText_(font, label), volumeText_(font, ""), font_(font) {

        // Label text
        labelText_.setCharacterSize(18);
        labelText_.setFillColor(sf::Color(200, 200, 200));

        // Volume percentage text
        volumeText_.setCharacterSize(18);
        volumeText_.setFillColor(sf::Color(200, 200, 200));

        // Background (track)
        background_.setSize({ BAR_WIDTH, BAR_HEIGHT });
        background_.setFillColor(sf::Color(30, 30, 30));
        background_.setOutlineColor(sf::Color(100, 100, 100));
        background_.setOutlineThickness(1.5f);

        // Fill (progress)
        fill_.setSize({ BAR_WIDTH * (volume_ / maxVolume_), BAR_HEIGHT });
        fill_.setFillColor(sf::Color(200, 200, 200));

        // Knob (slider handle)
        knob_.setSize({ 15.f, BAR_HEIGHT + 10.f });
        knob_.setFillColor(sf::Color(255, 255, 255));
        knob_.setOutlineColor(sf::Color(255, 255, 255));
        knob_.setOutlineThickness(1.5f);
    }

    void VolumeBar::setPosition(sf::Vector2f pos) {
        background_.setPosition(pos);
        fill_.setPosition(pos);

        float knobX = pos.x + (volume_ / maxVolume_) * BAR_WIDTH - 7.5f;
        knob_.setPosition({ knobX, pos.y - 5.f });

        labelText_.setPosition({ pos.x - 150.f, pos.y + 2.f });

        // Position volume percentage text to the right of the bar
        volumeText_.setPosition({ pos.x + BAR_WIDTH + 20.f, pos.y + 2.f });
    }

    void VolumeBar::setVolume(float vol) {
        float oldVolume = volume_;
        volume_ = std::max(minVolume_, std::min(maxVolume_, vol));

        // Update fill width
        fill_.setSize({ BAR_WIDTH * (volume_ / maxVolume_), BAR_HEIGHT });

        // Update knob position
        auto bgPos = background_.getPosition();
        float knobX = bgPos.x + (volume_ / maxVolume_) * BAR_WIDTH - 7.5f;
        knob_.setPosition({ knobX, bgPos.y - 5.f });

        // Update volume text
        volumeText_.setString(std::to_string(static_cast<int>(volume_)) + "%");

        // Trigger callback if volume changed
        if (oldVolume != volume_ && onVolumeChange) {
            onVolumeChange(volume_);
        }
    }

    void VolumeBar::handleMousePress(sf::Vector2f mousePos) {
        if (isHovered(mousePos)) {
            isDragging_ = true;
            handleMouseMove(mousePos);
        }
    }

    void VolumeBar::handleMouseRelease() {
        isDragging_ = false;
    }

    void VolumeBar::handleMouseMove(sf::Vector2f mousePos) {
        if (isDragging_) {
            auto bgPos = background_.getPosition();
            float relativeX = mousePos.x - bgPos.x;
            float newVolume = (relativeX / BAR_WIDTH) * maxVolume_;
            setVolume(newVolume);
        }
    }

    bool VolumeBar::isHovered(sf::Vector2f mousePos) const {
        sf::FloatRect bounds = background_.getGlobalBounds();
        bounds.position.y -= 10.f;
        bounds.size.y += 20.f;
        bounds.position.x -= 50.f;
        bounds.size.x += 100.f;
        return bounds.contains(mousePos);
    }

    void VolumeBar::forceStopDragging() {
        isDragging_ = false;
    }

    void VolumeBar::update() {
        // Could add animation or other updates here
    }

    void VolumeBar::draw(sf::RenderWindow& window) {
        window.draw(labelText_);
        window.draw(background_);
        window.draw(fill_);
        window.draw(knob_);
        window.draw(volumeText_);
    }

    // ─────────────────────────────────────────────────────────────────

    Settings::Settings(sf::RenderWindow& window, const sf::Font& font)
        : masterVolumeBar_(font, "Master Volume"),
          effectVolumeBar_(font, "Effect Volume"),
          font_(const_cast<sf::Font&>(font)),
          window_(window) {

        auto size = window_.getSize();
        float centerX = static_cast<float>(size.x) / 2.f;
        float centerY = static_cast<float>(size.y) / 2.f;

        // Position volume bars with better spacing
        masterVolumeBar_.setPosition({ centerX - 150.f, centerY - 60.f });
        effectVolumeBar_.setPosition({ centerX - 150.f, centerY + 40.f });

        // Initialize overlay
        overlay_.setSize({ static_cast<float>(size.x), static_cast<float>(size.y) });
        overlay_.setFillColor(sf::Color(0, 0, 0, 160));

        // Wire up volume bar callbacks to trigger Settings callbacks
        masterVolumeBar_.onVolumeChange = [this](float vol) {
            if (onMasterVolumeChange) {
                onMasterVolumeChange(vol);
            }
            saveSettings();
        };

        effectVolumeBar_.onVolumeChange = [this](float vol) {
            if (onEffectVolumeChange) {
                onEffectVolumeChange(vol);
            }
            saveSettings();
        };
    }

    void Settings::initialize() {
        // Load saved settings after callbacks are wired up
        loadSettings();
    }

    void Settings::handleInput(const sf::Event& event) {
        if (!showSettings_) {
            forceStopAllDragging();
            return;
        }

        if (event.is<sf::Event::MouseButtonPressed>()) {
            const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>();
            if (mouse->button == sf::Mouse::Button::Left) {
                auto mousePos = window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
                masterVolumeBar_.handleMousePress(mousePos);
                effectVolumeBar_.handleMousePress(mousePos);
            }
        }
        else if (event.is<sf::Event::MouseButtonReleased>()) {
            const auto* mouse = event.getIf<sf::Event::MouseButtonReleased>();
            if (mouse->button == sf::Mouse::Button::Left) {
                masterVolumeBar_.handleMouseRelease();
                effectVolumeBar_.handleMouseRelease();
            }
        }
    }

    void Settings::handleMouseMove(sf::Vector2f mousePos) {
        if (!showSettings_) {
            forceStopAllDragging();
            return;
        }

        masterVolumeBar_.handleMouseMove(mousePos);
        effectVolumeBar_.handleMouseMove(mousePos);
    }

    void Settings::handleMousePress(sf::Vector2f mousePos) {
        if (!showSettings_) return;
        
        masterVolumeBar_.handleMousePress(mousePos);
        effectVolumeBar_.handleMousePress(mousePos);
    }

    void Settings::handleMouseRelease() {
        masterVolumeBar_.handleMouseRelease();
        effectVolumeBar_.handleMouseRelease();
    }

    void Settings::update() {
        if (!showSettings_) {
            forceStopAllDragging();
            return;
        }

        masterVolumeBar_.update();
        effectVolumeBar_.update();
    }

    void Settings::draw() {
        if (!showSettings_) return;

        // Draw overlay to darken background
        window_.draw(overlay_);

        // Title
        sf::Text title(font_, "SETTINGS");
        title.setCharacterSize(36);
        title.setFillColor(sf::Color(255, 255, 255));

        auto size = window_.getSize();
        float centerX = static_cast<float>(size.x) / 2.f;
        auto bounds = title.getLocalBounds();
        title.setPosition({ centerX - bounds.size.x / 2.f, static_cast<float>(size.y) / 2.f - 180.f });

        window_.draw(title);

        // Draw volume bars
        masterVolumeBar_.draw(window_);
        effectVolumeBar_.draw(window_);

        // Draw back button
        sf::Text backText(font_, "Press ESC to go back");
        backText.setCharacterSize(14);
        backText.setFillColor(sf::Color(160, 160, 160));
        backText.setStyle(sf::Text::Italic);

        bounds = backText.getLocalBounds();
        backText.setPosition({ centerX - bounds.size.x / 2.f, static_cast<float>(size.y) - 80.f });
        window_.draw(backText);
    }

    void Settings::resetState() {
        forceStopAllDragging();
    }

    void Settings::forceStopAllDragging() {
        masterVolumeBar_.forceStopDragging();
        effectVolumeBar_.forceStopDragging();
    }

    void Settings::setMasterVolume(float vol) {
        masterVolumeBar_.setVolume(vol);
    }

    void Settings::setEffectVolume(float vol) {
        effectVolumeBar_.setVolume(vol);
    }

    void Settings::saveSettings() {
        // Get Documents folder path
        wchar_t* documentsPath = nullptr;
        if (SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &documentsPath) != S_OK) {
            std::cerr << "Failed to get Documents folder path\n";
            return;
        }

        // Convert wide string to regular string
        std::wstring wPath(documentsPath);
        CoTaskMemFree(documentsPath);
        std::string docPath(wPath.begin(), wPath.end());

        // Create CoreZone/Saved directory path
        std::string savePath = docPath + "\\CoreZone\\Saved";
        
        // Create directories if they don't exist
        try {
            std::filesystem::create_directories(savePath);
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to create save directory: " << e.what() << "\n";
            return;
        }

        // Save settings to file
        std::string filePath = savePath + "\\saved.log";
        std::ofstream file(filePath);
        if (file.is_open()) {
            file << "MasterVolume=" << masterVolumeBar_.getVolume() << "\n";
            file << "EffectVolume=" << effectVolumeBar_.getVolume() << "\n";
            file.close();
            std::cout << "Settings saved to: " << filePath << "\n";
        }
        else {
            std::cerr << "Failed to save settings to: " << filePath << "\n";
        }
    }

    void Settings::loadSettings() {
        // Get Documents folder path
        wchar_t* documentsPath = nullptr;
        if (SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &documentsPath) != S_OK) {
            std::cerr << "Failed to get Documents folder path\n";
            return;
        }

        // Convert wide string to regular string
        std::wstring wPath(documentsPath);
        CoTaskMemFree(documentsPath);
        std::string docPath(wPath.begin(), wPath.end());

        // Build file path
        std::string filePath = docPath + "\\CoreZone\\Saved\\saved.log";

        // Check if file exists
        if (!std::filesystem::exists(filePath)) {
            std::cout << "No saved settings found, using defaults\n";
            // Apply default volumes to audio
            if (onMasterVolumeChange) onMasterVolumeChange(masterVolumeBar_.getVolume());
            if (onEffectVolumeChange) onEffectVolumeChange(effectVolumeBar_.getVolume());
            return;
        }

        // Load settings from file
        std::ifstream file(filePath);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                size_t pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);

                    try {
                        float vol = std::stof(value);
                        if (key == "MasterVolume") {
                            masterVolumeBar_.setVolume(vol);
                            if (onMasterVolumeChange) onMasterVolumeChange(vol);
                        }
                        else if (key == "EffectVolume") {
                            effectVolumeBar_.setVolume(vol);
                            if (onEffectVolumeChange) onEffectVolumeChange(vol);
                        }
                    }
                    catch (const std::exception& e) {
                        std::cerr << "Failed to parse volume value: " << e.what() << "\n";
                    }
                }
            }
            file.close();
            std::cout << "Settings loaded from: " << filePath << "\n";
        }
        else {
            std::cerr << "Failed to load settings from: " << filePath << "\n";
        }
    }

} // namespace corezone
