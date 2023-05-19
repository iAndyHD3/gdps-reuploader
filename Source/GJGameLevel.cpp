#include "GJGameLevel.h"
#include <string>
#include <map>
#include <vector>
#include <charconv>

//the only thing we actually want as normal string is the class members
static inline std::string _toString(std::string_view s) {
	return {s.begin(), s.end()};
}


std::vector<std::string_view> splitByDelimStringView(std::string_view str, char delim)
{
    std::vector<std::string_view> tokens;
    size_t pos = 0;
    size_t len = str.length();

    while (pos < len) {
        size_t end = str.find(delim, pos);
        if (end == std::string_view::npos) {
            tokens.emplace_back(str.substr(pos));
            break;
        }
        tokens.emplace_back(str.substr(pos, end - pos));
        pos = end + 1;
    }

    return tokens;
}

int _stoi(const std::string_view s) {
	int ret = 0;
	std::from_chars(s.data(),s.data() + s.size(), ret);
	return ret;
}

GJGameLevel GJGameLevel::createWithResponse(std::string_view backendResponse)
{
	GJGameLevel level;
	if(backendResponse.starts_with('-')) {
		level._errorStr = _toString(backendResponse);
		return level;
	}
	
	level.setFromBackendResponse(backendResponse);
	return level;
}

void GJGameLevel::setFromBackendResponse(std::string_view backendResponse)
{
	auto stuff = splitByDelimStringView(backendResponse, ':');

	std::map<std::string_view, std::string_view> levelResponse;

	for (size_t i = 0; i < stuff.size(); i += 2)
	{
		if (auto val = stuff[i + 1]; !val.empty())
			levelResponse.insert({stuff[i], val});
	}
	
	if (levelResponse.contains("1")) _levelID = _stoi(levelResponse["1"]);
	if (levelResponse.contains("2")) _levelName = _toString(levelResponse["2"]);
	if (levelResponse.contains("3")) _description = _toString(levelResponse["3"]);
	if (levelResponse.contains("4")) _levelString = _toString(levelResponse["4"]);
	if (levelResponse.contains("5")) _version = _stoi(levelResponse["5"]);
	if (levelResponse.contains("6")) _playerID = _stoi(levelResponse["6"]);
	if (levelResponse.contains("8")) _difficultyDenominator = _stoi(levelResponse["8"]);
	if (levelResponse.contains("9")) _difficultyNumerator = _stoi(levelResponse["9"]);
	if (levelResponse.contains("10")) _downloads = _stoi(levelResponse["10"]);
	if (levelResponse.contains("11")) _setCompletes = _stoi(levelResponse["11"]);
	if (levelResponse.contains("12")) _officialSongID = _stoi(levelResponse["12"]);
	if (levelResponse.contains("13")) _gameVersion = _stoi(levelResponse["13"]);
	if (levelResponse.contains("14")) _likes = _stoi(levelResponse["14"]);
	if (levelResponse.contains("15")) _length = _stoi(levelResponse["15"]);
	if (levelResponse.contains("16")) _dislikes = _stoi(levelResponse["16"]);
	if (levelResponse.contains("17")) _demon = _stoi(levelResponse["17"]);
	if (levelResponse.contains("18")) _stars = _stoi(levelResponse["18"]);
	if (levelResponse.contains("19")) _featureScore = _stoi(levelResponse["19"]);
	if (levelResponse.contains("25")) _auto = _stoi(levelResponse["25"]);
	if (levelResponse.contains("26")) _recordString = _toString(levelResponse["26"]);
	if (levelResponse.contains("28")) _uploadDate = _toString(levelResponse["28"]);
	if (levelResponse.contains("29")) _updateDate = _toString(levelResponse["29"]);
	if (levelResponse.contains("30")) _copiedID = _stoi(levelResponse["30"]);
	if (levelResponse.contains("31")) _2P = _stoi(levelResponse["31"]);
	if (levelResponse.contains("35")) _songID = _stoi(levelResponse["35"]);
	if (levelResponse.contains("37")) _coins = _stoi(levelResponse["37"]);
	if (levelResponse.contains("38")) _verifiedCoins = _stoi(levelResponse["38"]);
	if (levelResponse.contains("39")) _starsRequested = _stoi(levelResponse["39"]);
	if (levelResponse.contains("40")) _LDM = _stoi(levelResponse["40"]);
	if (levelResponse.contains("41")) _dailyNumber = _stoi(levelResponse["41"]);
	if (levelResponse.contains("42")) _epic = _stoi(levelResponse["42"]);
	if (levelResponse.contains("43")) _demonDifficulty = _stoi(levelResponse["43"]);
	if (levelResponse.contains("44")) _gauntlet = _stoi(levelResponse["44"]);
	if (levelResponse.contains("45")) _objects = _stoi(levelResponse["45"]);
	if (levelResponse.contains("46")) _editorTime = _stoi(levelResponse["46"]);
	if (levelResponse.contains("47")) _editorTimeTotal = _stoi(levelResponse["47"]);
	
}