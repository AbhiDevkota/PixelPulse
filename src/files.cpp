#include "files.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <ctime>

namespace corezone {

	//Assests implementation
	const std::string AssetPath::ASSETS_DIR = "assets/";
	const std::string AssetPath::FONTS_DIR = "fonts/";
	const std::string AssetPath::AUDIOS_DIR = "audios/";
	const std::string AssetPath::ICONS_DIR = "Icons/";
	const std::string AssetPath::CONFIG_DIR = "config/";

	std::string AssetPath::getAssetPath(const std::string& assetName) {
		return ASSETS_DIR + assetName;
	}

	std::string AssetPath::getFontPath(const std::string& fontName) {
		return FONTS_DIR + fontName;
	}

	std::string AssetPath::getAudioPath(const std::string& audioName) {
		return AUDIOS_DIR + audioName;
	}

	std::string AssetPath::getIconPath(const std::string& iconName) {
		return ICONS_DIR + iconName;
	}

	std::string AssetPath::getConfigPath(const std::string& configName) {
		return CONFIG_DIR + configName;
	}

	bool AssetPath::assetExists(const std::string& assetPath) {
		return std::filesystem::exists(assetPath);
	}

	std::vector<std::string> AssetPath::getAvailableFonts() {
		std::vector<std::string> fonts;
		try {
			if (std::filesystem::exists(FONTS_DIR)) {
				for (const auto& entry : std::filesystem::directory_iterator(FONTS_DIR)) {
					if (entry.is_regular_file()) {
						fonts.push_back(entry.path().filename().string());
					}
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error reading fonts directory: " << e.what() << std::endl;
		}
		return fonts;
	}

	std::vector<std::string> AssetPath::getAvailableAudios() {
		std::vector<std::string> audios;
		try {
			if (std::filesystem::exists(AUDIOS_DIR)) {
				for (const auto& entry : std::filesystem::directory_iterator(AUDIOS_DIR)) {
					if (entry.is_regular_file()) {
						audios.push_back(entry.path().filename().string());
					}
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error reading audios directory: " << e.what() << std::endl;
		}
		return audios;
	}

	std::vector<std::string> AssetPath::getAvailableIcons() {
		std::vector<std::string> icons;
		try {
			if (std::filesystem::exists(ICONS_DIR)) {
				for (const auto& entry : std::filesystem::directory_iterator(ICONS_DIR)) {
					if (entry.is_regular_file()) {
						icons.push_back(entry.path().filename().string());
					}
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error reading icons directory: " << e.what() << std::endl;
		}
		return icons;
	}

	//file mgt implementation

	const std::string FileManager::DEFAULT_CONFIG_FILE = "config/settings.cfg";
	const std::string FileManager::AUDIO_CONFIG_FILE = "config/audio.cfg";

	FileManager::FileManager() : configFilePath_(DEFAULT_CONFIG_FILE), audioConfigFilePath_(AUDIO_CONFIG_FILE) {
	}

	bool FileManager::initialize() {
		//Create config directory if dont have
		if (!createDirectory("config")) {
			std::cerr << "Failed to create config directory" << std::endl;
			return false;
		}

		//try to load existing config
		if (fileExists(configFilePath_)) {
			if (!loadConfig()) {
				std::cerr << "Warning: Failed to load config file, creating new one" << std::endl;
			}
		}
		else {
			std::cout << "No existing config found, creating new one" << std::endl;
		}

		return true;
	}

	bool FileManager::createDirectory(const std::string& path) {
		try {
			if (!std::filesystem::exists(path)) {
				return std::filesystem::create_directories(path);
			}
			return true;
		}
		catch (const std::exception& e) {
			std::cerr << "Error creating directory: " << e.what() << std::endl;
			return false;
		}
	}

	bool FileManager::fileExists(const std::string& filePath) const {
		try {
			return std::filesystem::exists(filePath);
		}
		catch (const std::exception& e) {
			std::cerr << "Error checking file existence: " << e.what() << std::endl;
			return false;
		}
	}

	bool FileManager::readFile(const std::string& filePath, std::string& content) const {
		if (!fileExists(filePath)) {
			std::cerr << "File not found: " << filePath << std::endl;
			return false;
		}

		try {
			std::ifstream file(filePath);
			if (!file.is_open()) {
				std::cerr << "Failed to open file: " << filePath << std::endl;
				return false;
			}

			std::stringstream buffer;
			buffer << file.rdbuf();
			content = buffer.str();
			file.close();

			return true;
		}
		catch (const std::exception& e) {
			std::cerr << "Error reading file: " << e.what() << std::endl;
			return false;
		}
	}

	bool FileManager::writeFile(const std::string& filePath, const std::string& content) {
		try {
			//create directory if needed
			std::string dir = getDirectoryPath(filePath);
			if (!dir.empty() && !fileExists(dir)) {
				createDirectory(dir);
			}

			std::ofstream file(filePath);
			if (!file.is_open()) {
				std::cerr << "Failed to open file for writing: " << filePath << std::endl;
				return false;
			}

			file << content;
			file.close();

			return true;
		}
		catch (const std::exception& e) {
			std::cerr << "Error writing file: " << e.what() << std::endl;
			return false;
		}
	}

	bool FileManager::appendToFile(const std::string& filePath, const std::string& content) {
		try {
			//create file if it doesn't exist
			if (!fileExists(filePath)) {
				return writeFile(filePath, content);
			}

			std::ofstream file(filePath, std::ios::app);
			if (!file.is_open()) {
				std::cerr << "Failed to open file for appending: " << filePath << std::endl;
				return false;
			}

			file << content;
			file.close();

			return true;
		}
		catch (const std::exception& e) {
			std::cerr << "Error appending to file: " << e.what() << std::endl;
			return false;
		}
	}

	bool FileManager::deleteFile(const std::string& filePath) const {
		try {
			if (!fileExists(filePath)) {
				std::cerr << "File not found: " << filePath << std::endl;
				return false;
			}
			return std::filesystem::remove(filePath) > 0;
		}
		catch (const std::exception& e) {
			std::cerr << "Error deleting file: " << e.what() << std::endl;
			return false;
		}
	}

	void FileManager::setConfigValue(const std::string& key, const std::string& value) {
		configData_[key] = value;
	}

	std::string FileManager::getConfigValue(const std::string& key, const std::string& defaultValue) const {
		auto it = configData_.find(key);
		if (it != configData_.end()) {
			return it->second;
		}
		return defaultValue;
	}

	bool FileManager::hasConfigKey(const std::string& key) const {
		return configData_.find(key) != configData_.end();
	}

	bool FileManager::saveConfig() {
		try {
			std::stringstream buffer;

			//Write header
			buffer << "# PixelPulse Configuration File" << std::endl;
			buffer << "# Generated at runtime" << std::endl;
			buffer << std::endl;

			//Write all key-value pairs
			for (const auto& [key, value] : configData_) {
				buffer << key << "=" << value << std::endl;
			}

			return writeFile(configFilePath_, buffer.str());
		}
		catch (const std::exception& e) {
			std::cerr << "Error saving config: " << e.what() << std::endl;
			return false;
		}
	}

	bool FileManager::loadConfig() {
		try {
			std::string content;
			if (!readFile(configFilePath_, content)) {
				return false;
			}

			configData_.clear();

			std::stringstream stream(content);
			std::string line;

			while (std::getline(stream, line)) {
				//Skip comments and empty lines
				if (line.empty() || line[0] == '#') {
					continue;
				}

				//Parse key=value
				size_t delimPos = line.find('=');
				if (delimPos != std::string::npos) {
					std::string key = line.substr(0, delimPos);
					std::string value = line.substr(delimPos + 1);

					//Trim whitespace
					key.erase(0, key.find_first_not_of(" \t\r\n"));
					key.erase(key.find_last_not_of(" \t\r\n") + 1);
					value.erase(0, value.find_first_not_of(" \t\r\n"));
					value.erase(value.find_last_not_of(" \t\r\n") + 1);

					configData_[key] = value;
				}
			}

			return true;
		}
		catch (const std::exception& e) {
			std::cerr << "Error loading config: " << e.what() << std::endl;
			return false;
		}
	}

	bool FileManager::resetConfig() {
		configData_.clear();
		return deleteFile(configFilePath_);
	}

	bool FileManager::saveVolumeData(const std::string& volumeType, float volume){
		float clampedVolume = std::max(0.0f, std::min(100.0f, volume));
		audioData_[volumeType] = clampedVolume;
		return true;
	}

	bool FileManager::loadVolumeData(const std::string& volumeType, float& volume) {
		auto it = audioData_.find(volumeType);
		if (it != audioData_.end()) {
			volume = it->second;
			return true;
		}
		volume = 70.0f;
		return false;
	}

	float FileManager::getVolumeData(const std::string& volumeType, float defaultVolume) const {
		auto it = audioData_.find(volumeType);
		if (it != audioData_.end()) {
			return it->second;
		}
		return defaultVolume;
	}

	void FileManager::setVolumeData(const std::string& volumeType, float volume) {
		float clampedVolume = std::max(0.0f, std::min(100.0f, volume));
		audioData_[volumeType] = clampedVolume;
	}

	bool FileManager::saveAllVolumeData() {
		try {
			std::stringstream buffer;

			// Write header
			buffer << "# PixelPulse Audio Configuration File" << std::endl;
			buffer << "# Volume levels (0-100)" << std::endl;
			buffer << std::endl;

			//Write all volume entries
			for (const auto& [volumeType, volume] : audioData_) {
				buffer << volumeType << "=" << volume << std::endl;
			}

			return writeFile(audioConfigFilePath_, buffer.str());
		}
		catch (const std::exception& e) {
			std::cerr << "Error saving audio config: " << e.what() << std::endl;
			return false;
		}
	}

	bool FileManager::loadAllVolumeData() {
		try {
			std::string content;
			if (!readFile(audioConfigFilePath_, content)) {
				return false;
			}

			audioData_.clear();

			std::stringstream stream(content);
			std::string line;

			while (std::getline(stream, line)) {
				// Skip comments and empty lines
				if (line.empty() || line[0] == '#') {
					continue;
				}

				// Parse key=value
				size_t delimPos = line.find('=');
				if (delimPos != std::string::npos) {
					std::string key = line.substr(0, delimPos);
					std::string valueStr = line.substr(delimPos + 1);

					// Trim whitespace
					key.erase(0, key.find_first_not_of(" \t\r\n"));
					key.erase(key.find_last_not_of(" \t\r\n") + 1);
					valueStr.erase(0, valueStr.find_first_not_of(" \t\r\n"));
					valueStr.erase(valueStr.find_last_not_of(" \t\r\n") + 1);

					try {
						float volume = std::stof(valueStr);
						// Clamp volume between 0 and 100
						volume = std::max(0.0f, std::min(100.0f, volume));
						audioData_[key] = volume;
					}
					catch (const std::exception& e) {
						std::cerr << "Error parsing volume value for " << key << ": " << e.what() << std::endl;
					}
				}
			}

			return true;
		}
		catch (const std::exception& e) {
			std::cerr << "Error loading audio config: " << e.what() << std::endl;
			return false;
		}
	}

	bool FileManager::resetVolumeData() {
		audioData_.clear();
		return deleteFile(audioConfigFilePath_);
	}

	bool FileManager::saveGameData(const std::string& gameName, const std::string& gameData) {
		std::string fileName = "config/" + gameName + "_data.sav";
		return writeFile(fileName, gameData);
	}

	bool FileManager::loadGameData(const std::string& gameName, std::string& gameData) {
		std::string fileName = "config/" + gameName + "_data.sav";
		return readFile(fileName, gameData);
	}

	bool FileManager::deleteGameData(const std::string& gameName) {
		std::string fileName = "config/" + gameName + "_data.sav";
		return deleteFile(fileName);
	}

	std::string FileManager::getFileExtension(const std::string& filePath) {
		size_t dotPos = filePath.find_last_of('.');
		if (dotPos != std::string::npos && dotPos < filePath.length() - 1) {
			return filePath.substr(dotPos + 1);
		}
		return "";
	}

	std::string FileManager::getFileName(const std::string& filePath) {
		try {
			return std::filesystem::path(filePath).filename().string();
		}
		catch (const std::exception& e) {
			std::cerr << "Error getting filename: " << e.what() << std::endl;
			return "";
		}
	}

	std::string FileManager::getDirectoryPath(const std::string& filePath) {
		try {
			return std::filesystem::path(filePath).parent_path().string();
		}
		catch (const std::exception& e) {
			std::cerr << "Error getting directory path: " << e.what() << std::endl;
			return "";
		}
	}

	long FileManager::getFileSize(const std::string& filePath) {
		try {
			if (std::filesystem::exists(filePath)) {
				return std::filesystem::file_size(filePath);
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error getting file size: " << e.what() << std::endl;
		}
		return -1;
	}

	//game mgt implementation

	GameDataManager::GameDataManager(FileManager& fileManager, const std::string& gameName)
		: fileManager_(fileManager), gameName_(gameName) {
	}

	bool GameDataManager::saveHighScore(int score) {
		std::stringstream ss;
		ss << score;
		return fileManager_.saveGameData(gameName_ + "_highscore", ss.str());
	}

	bool GameDataManager::getHighScore(int& score) const {
		std::string data;
		if (fileManager_.loadGameData(gameName_ + "_highscore", data)) {
			try {
				score = std::stoi(data);
				return true;
			}
			catch (...) {
				score = 0;
				return false;
			}
		}
		score = 0;
		return false;
	}

	bool GameDataManager::saveGameStats(int gamesPlayed, int gamesWon, int totalScore) {
		std::stringstream ss;
		ss << gamesPlayed << "," << gamesWon << "," << totalScore;
		return fileManager_.saveGameData(gameName_ + "_stats", ss.str());
	}

	bool GameDataManager::loadGameStats(int& gamesPlayed, int& gamesWon, int& totalScore) const {
		std::string data;
		if (fileManager_.loadGameData(gameName_ + "_stats", data)) {
			try {
				std::stringstream ss(data);
				std::string field;

				std::getline(ss, field, ',');
				gamesPlayed = std::stoi(field);

				std::getline(ss, field, ',');
				gamesWon = std::stoi(field);

				std::getline(ss, field, ',');
				totalScore = std::stoi(field);

				return true;
			}
			catch (...) {
				gamesPlayed = gamesWon = totalScore = 0;
				return false;
			}
		}
		gamesPlayed = gamesWon = totalScore = 0;
		return false;
	}

	bool GameDataManager::saveGameState(const std::string& gameState) {
		return fileManager_.saveGameData(gameName_ + "_state", gameState);
	}

	bool GameDataManager::loadGameState(std::string& gameState) const {
		return fileManager_.loadGameData(gameName_ + "_state", gameState);
	}

	bool GameDataManager::saveLastSessionInfo(const std::string& sessionInfo) {
		return fileManager_.saveGameData(gameName_ + "_session", sessionInfo);
	}

	bool GameDataManager::loadLastSessionInfo(std::string& sessionInfo) const {
		return fileManager_.loadGameData(gameName_ + "_session", sessionInfo);
	}

}
