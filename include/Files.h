#ifndef FILES_H
#define FILES_H

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace corezone {

	//assests mgt system
	class AssetPath {
	private:
		static const std::string ASSETS_DIR;
		static const std::string FONTS_DIR;
		static const std::string AUDIOS_DIR;
		static const std::string ICONS_DIR;
		static const std::string CONFIG_DIR;

	public:
		//get path for assests type. 
		static std::string getAssetPath(const std::string& assetName);
		static std::string getFontPath(const std::string& fontName);
		static std::string getAudioPath(const std::string& audioName);
		static std::string getIconPath(const std::string& iconName);
		static std::string getConfigPath(const std::string& configName);

		//check if assests wxits
		static bool assetExists(const std::string& assetPath);

		//get assests and dont loct
		static std::vector<std::string> getAvailableFonts();
		static std::vector<std::string> getAvailableAudios();
		static std::vector<std::string> getAvailableIcons();
	};

	//File mgpt. Create, edit read, reset
	class FileManager {
	private:
		static const std::string DEFAULT_CONFIG_FILE;
		static const std::string AUDIO_CONFIG_FILE;		//Audo 
		std::string configFilePath_;
		std::string audioConfigFilePath_;
		std::unordered_map<std::string, std::string> configData_;
		std::unordered_map<std::string, float> audioData_;

	public:
		FileManager();
		~FileManager() = default;

		//init file mgt and load data
		bool initialize();

		//create directory if not exist
		static bool createDirectory(const std::string& path);

		//read write operations
		bool fileExists(const std::string& filePath) const;
		bool readFile(const std::string& filePath, std::string& content) const;
		bool writeFile(const std::string& filePath, const std::string& content);
		bool appendToFile(const std::string& filePath, const std::string& content);
		bool deleteFile(const std::string& filePath) const;

		//config mgt
		void setConfigValue(const std::string& key, const std::string& value);
		std::string getConfigValue(const std::string& key, const std::string& defaultValue = "") const;
		bool hasConfigKey(const std::string& key) const;

		//save, load. reset config data
		bool saveConfig();
		bool loadConfig();
		bool resetConfig();

		//Audio config mgt
		bool saveVolumeData(const std::string& volumeType, float volume);
		bool loadVolumeData(const std::string& volumeType, float& volume);
		float getVolumeData(const std::string& volumeType, float defaultVolume = 70.0f) const;
		void setVolumeData(const std::string& volumeType, float volume);
		bool saveAllVolumeData();
		bool loadAllVolumeData();
		const std::unordered_map<std::string, float>& getAllVolumeData() const { return audioData_; }

		//game save data mgt
		bool saveGameData(const std::string& gameName, const std::string& gameData);
		bool loadGameData(const std::string& gameName, std::string& gameData);
		bool deleteGameData(const std::string& gameName);

		//utility Functions
		static std::string getFileExtension(const std::string& filePath);
		static std::string getFileName(const std::string& filePath);
		static std::string getDirectoryPath(const std::string& filePath);
		static long getFileSize(const std::string& filePath);

		//extract all config data as a map
		const std::unordered_map<std::string, std::string>& getAllConfig() const { return configData_; }
	};

	//Handle game speciific saved data "high scores etc"
	class GameDataManager {
	private:
		FileManager& fileManager_;
		std::string gameName_;

	public:
		GameDataManager(FileManager& fileManager, const std::string& gameName);

		//high score mgt
		bool saveHighScore(int score);
		bool getHighScore(int& score) const;

		//game stats
		bool saveGameStats(int gamesPlayed, int gamesWon, int totalScore);
		bool loadGameStats(int& gamesPlayed, int& gamesWon, int& totalScore) const;

		//level info
		bool saveGameState(const std::string& gameState);
		bool loadGameState(std::string& gameState) const;

		//last save
		bool saveLastSessionInfo(const std::string& sessionInfo);
		bool loadLastSessionInfo(std::string& sessionInfo) const;
	};

}

#endif
