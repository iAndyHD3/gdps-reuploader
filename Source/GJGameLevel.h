#pragma once
#include <string>
#include <string_view>


class GJGameLevel {
public:

	//initialize everything otherwise some stuff just gets set to strange values idk

	int _levelID = 0;
	int _version = 0;
	int _playerID = 0;
	int _downloads = 0;
	int _musicID = 0;
	int _likes = 0;
	int _length = 0;
	int _difficultyDenominator = 0;
	int _difficultyNumerator = 0;
	int _setCompletes = 0;
	int _gameVersion = 0;
	int _dislikes = 0;
	int _stars = 0;
	int _featureScore = 0;
	int _copiedID = 0;
	int _coins = 0;
	int _starsRequested = 0;
	int _dailyNumber = 0;
	int _epic = 0;
	int _demonDifficulty = 0;
	int _objects = 0;
	int _editorTime = 0;
	int _editorTimeTotal = 0;
	int _songID = 0;
	int _officialSongID = 0;

	float _normalPercent = 0.f;
	float _practicePercent = 0.f;
	
	int _auto = false;
	int _demon = false;
	int _verifiedCoins = false;
	int _LDM = false;
	int _gauntlet = false;
	int _2P = false;

	std::string _levelName{};
	std::string _description{};
	std::string _levelString{};
	std::string _XORPassword{};
	std::string _uploadDate{};
	std::string _updateDate{};
	std::string _extraString{};
	std::string _settings{};
	std::string _recordString{};
	std::string _sondURL{};
	std::string _songName{};
	std::string _levelCreator{};
	
	//added
	std::string _errorStr{};
	
	void setFromBackendResponse(std::string_view);

	// Expects RobTop like string. If it not, nullptr gets returned.
	static GJGameLevel createWithResponse(std::string_view backendResponse);

};
