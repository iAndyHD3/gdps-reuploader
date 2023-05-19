#include "network/HttpRequest.h"
#include "network/HttpClient.h"
#include "base/CCDirector.h"
#include "2d/CCScene.h"
#include "2d/CCLabel.h"
#include "GJGameLevel.h"
#include "sha1.hpp"

#include <string>
#include <string_view>
#include <optional>
#include <fmt/format.h>

//#define USE_WIN32_CONSOLE


template <typename... T> 
void fmt_print(fmt::format_string<T...> fmt, T&&... args) {
	#ifdef USE_WIN32_CONSOLE
	fmt::print(fmt, args);
	#endif
}


constexpr int REUPLOAD_OK = 0;
constexpr int GDPS_LOGIN_ERROR = 1;
constexpr int GD_LOGIN_ERROR = 2;
constexpr int INVALID_LEVELID = 3;
constexpr int LEVELID_OTHER_ACCOUNT = 4;
constexpr int UPLOAD_LEVEL_ERROR = 5;
constexpr int UNKNOWN_SERVER_ERROR = 6;


/*
-------- REUPLOAD STEPS IN ORDER -------
1. Login GDPS
2. Get level from GDPS
3. Login GD
4. Uplaod to GD
*/



//FORWARD DECLARE

//this is the order in which the reupload happens
//each one of these create on success create the next http request for the next handler
void onLoginGDPS(ax::network::HttpClient* sender, ax::network::HttpResponse* response);
void onGetGDPSLevel(ax::network::HttpClient* sender, ax::network::HttpResponse* response);
void onDownloadGDPSLevel(ax::network::HttpClient* sender, ax::network::HttpResponse* response);
void onLoginGD(ax::network::HttpClient* sender, ax::network::HttpResponse* response);
void onUploadLevelGD(ax::network::HttpClient* sender, ax::network::HttpResponse* response);

std::string generateUDID();
std::string to_gjp2(std::string_view plainTextPassword);
std::string to_gjp(std::string password);
std::string getSeed2(const GJGameLevel& level);

std::string getResponse(ax::network::HttpResponse* response);
ax::network::HttpRequest* createRequest(const std::string& url, const std::string& postData, const ax::network::ccHttpRequestCallback& callback);
std::string getPostDataForLevelUpload(const GJGameLevel&, bool unlisted);
void setLabelText(std::string_view text);

extern void callLuaResult(int result, const char* error); //AppDelegate.cpp
extern std::vector<std::string_view> splitByDelimStringView(std::string_view str, char delim); //GJGameLevel.cpp
extern int _stoi(const std::string_view s); //GJGameLevel.cpp




//DEFINITIONS
struct ReuploadInfo
{
	std::string gdpsUsername;
	std::string gdpsPassword;
	std::string gdUsername;
	std::string gdPassword;
	int levelID;
	int accID;
	int userID;
	int gd_accID;
	int gd_userID;
};

//globals
static ReuploadInfo RI;
static GJGameLevel LEVEL;

void reupload_main(
	std::string gdpsUsername,
	std::string gdpsPassword,
	std::string gdUsername,
	std::string gdPassword,
	int levelID
)
{
	RI.gdpsUsername = gdpsUsername;
	RI.gdpsPassword = gdpsPassword;
	RI.gdUsername = gdUsername;
	RI.gdPassword = gdPassword;
	RI.levelID = levelID;
	
	std::string gdpsShaPass = to_gjp2(gdpsPassword);
	fmt_print("sha pass: {}", gdpsShaPass);

	std::string postData = fmt::format("udid={}&userName={}&gjp2={}", generateUDID(), gdpsUsername, gdpsShaPass);
	fmt_print("post data: {}\n", postData);
	
	auto request = createRequest("http://game.gdpseditor.com/server/accounts/loginGJAccount.php", postData, onLoginGDPS);
	ax::network::HttpClient::getInstance()->send(request);
	request->release();
	setLabelText("Login GDPS...");
}

void onLoginGDPS(ax::network::HttpClient* sender, ax::network::HttpResponse* response)
{
	std::string strResponse = getResponse(response);
	fmt_print("strResponse: {}\n", strResponse);
	
	//error if: empty, starts with - (error) or does not contain comma (separator for acc id and user id)
	if(strResponse.empty() || strResponse.starts_with('-') || strResponse.find(',') == std::string::npos)
	{
		callLuaResult(GDPS_LOGIN_ERROR, strResponse.c_str());
		return;
	}
	
	std::vector<std::string_view> userinfo = splitByDelimStringView(strResponse, ',');
	int accID = _stoi(userinfo[0]);
	int userID = _stoi(userinfo[1]);
	
	RI.accID = accID;
	RI.userID = userID;
	
	
	std::string postData = fmt::format("str={}", RI.levelID);
	auto request = createRequest("http://game.gdpseditor.com/server/getGJLevels22.php", postData, onGetGDPSLevel);
	ax::network::HttpClient::getInstance()->send(request);
	request->release();
	setLabelText("Getting level info...");
}

void onGetGDPSLevel(ax::network::HttpClient* sender, ax::network::HttpResponse* response)
{
	std::string strResponse = getResponse(response);
	fmt_print("strResponse: {}\n", strResponse);
	
	//## = no level id was found + no levels were found
	//pipe (|) is the separator = more levels were found (invalid level ID)
	if(strResponse.empty() || strResponse.starts_with('-') || strResponse.starts_with("##") || strResponse.find('|') != std::string::npos)
	{
		if(strResponse.starts_with("##"))
			strResponse = "level does not exist";
		
		callLuaResult(INVALID_LEVELID, strResponse.c_str());
		return;
	}
	
	LEVEL.setFromBackendResponse(strResponse);
	
	if(LEVEL._playerID != RI.userID)
	{
		callLuaResult(LEVELID_OTHER_ACCOUNT, nullptr); //no error message
		return;
	}
	
	std::string postData = fmt::format("levelID={}", RI.levelID);
	
	auto request = createRequest("http://game.gdpseditor.com/server/downloadGJLevel22.php", postData, onDownloadGDPSLevel);
	ax::network::HttpClient::getInstance()->send(request);
	request->release();
	setLabelText("Downloading level...");
}

void onDownloadGDPSLevel(ax::network::HttpClient* sender, ax::network::HttpResponse* response)
{
	std::string strResponse = getResponse(response);
	fmt_print("strResponse: {}\n", strResponse);
	
	if(strResponse.empty() || strResponse.starts_with('-'))
	{
		callLuaResult(UNKNOWN_SERVER_ERROR, strResponse.c_str());
		return;
	}
	LEVEL.setFromBackendResponse(strResponse);
	
	std::string postData = fmt::format("udid={}&userName={}&password={}&secret=Wmfv3899gc9", generateUDID(), RI.gdUsername, RI.gdPassword);
	auto request = createRequest("http://www.boomlings.com/database/accounts/loginGJAccount.php", postData, onLoginGD);
	ax::network::HttpClient::getInstance()->send(request);
	request->release();
	setLabelText("Login GD...");
}

void onLoginGD(ax::network::HttpClient* sender, ax::network::HttpResponse* response)
{
	std::string strResponse = getResponse(response);
	fmt_print("strResponse: {}\n", strResponse);
	
	//error if: empty, starts with - (error) or does not contain comma (separator for acc id and user id)
	if(strResponse.empty() || strResponse.starts_with('-') || strResponse.find(',') == std::string::npos)
	{
		callLuaResult(GD_LOGIN_ERROR, strResponse.c_str());
		return;
	}
	
	std::vector<std::string_view> userinfo = splitByDelimStringView(strResponse, ',');
	int accID = _stoi(userinfo[0]);
	int userID = _stoi(userinfo[1]);
	
	RI.gd_accID = accID;
	RI.gd_userID = userID;
	
	std::string postData = getPostDataForLevelUpload(LEVEL, true);
	auto request = createRequest("http://www.boomlings.com/database/uploadGJLevel21.php", postData, onUploadLevelGD);
	ax::network::HttpClient::getInstance()->send(request);
	request->release();
	setLabelText("Uploading level...");
}

void onUploadLevelGD(ax::network::HttpClient* sender, ax::network::HttpResponse* response)
{
	std::string strResponse = getResponse(response);
	fmt_print("strResponse: {}\n", strResponse);
	
	//error if: empty, starts with - (error) or does not contain comma (separator for acc id and user id)
	if(strResponse.empty() || strResponse.starts_with('-'))
	{
		callLuaResult(UPLOAD_LEVEL_ERROR, strResponse.c_str());
		return;
	}
	
	int levelID = _stoi(strResponse);
	if(levelID > 10)
		return callLuaResult(levelID, nullptr);
	
	callLuaResult(UNKNOWN_SERVER_ERROR, fmt::format("levelID: {}", levelID).c_str());
}

std::string generateUDID() {
	auto rand_str = [](size_t length) -> std::string
	{
		auto randchar = []() -> char
		{
			const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
			const size_t max_index = (sizeof(charset) - 1);
			return charset[ rand() % max_index ];
		};
		std::string str(length,0);
		std::generate_n(str.begin(), length, randchar);
		return str;
	};
	return fmt::format("{}-{}-{}-{}-{}", rand_str(8), rand_str(4), rand_str(4), rand_str(4), rand_str(8));
}

std::string to_gjp2(std::string_view _plainTextPassword)
{
	std::string plainTextPassword {_plainTextPassword.begin(), _plainTextPassword.end()};
	plainTextPassword += "mI29fmAnxgTs";
	SHA1 s;
	s.update(plainTextPassword);
	return s.final();
}

std::string getResponse(ax::network::HttpResponse* response)
{
	auto buffer = response->getResponseData();
	std::string ret{buffer->begin(), buffer->end()};
	return ret;
}

//std::string to avoid null terminator issues
ax::network::HttpRequest* createRequest(const std::string& url, const std::string& postData, const ax::network::ccHttpRequestCallback& callback)
{
	fmt_print("URL: {}, POST: {}", url, postData);
	auto req = new ax::network::HttpRequest();
	req->setUrl(url.c_str());
	req->setRequestType(ax::network::HttpRequest::Type::POST);
	req->setHeaders(std::vector<std::string>{"User-Agent: "});
	req->setRequestData(postData.c_str(), postData.length());
	req->setResponseCallback(callback);
	req->setTag("valid");
	return req;
}

std::string getPostDataForLevelUpload(const GJGameLevel& level, bool unlisted)
{
	constexpr const char* fmtString = 
	"gameVersion=21&accountID={}&gjp={}&userName={}&"
	"levelID={}&levelName={}&levelDesc={}&"
	"levelVersion={}&levelLength={}&audioTrack={}&auto=0&"
	"password={}&original={}&twoPlayer={}&songID={}&"
	"objects={}&coins={}&requestedStars={}&unlisted={}&"
	"ldm={}&levelString={}&seed2={}&secret=Wmfd2893gb7";
	
	return fmt::format(fmtString,
	RI.gd_accID, to_gjp(RI.gdPassword), RI.gdUsername,
	0, level._levelName, level._description,
	level._version, level._length, level._officialSongID,
	0, RI.levelID, level._2P, level._songID,
	level._objects, level._coins, level._starsRequested, (int)unlisted,
	level._LDM, level._levelString, getSeed2(level));
}


static std::string base64_encode(const std::string &in) {
	std::string out;
	static const std::string_view base64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//=

	int val=0, valb=-6;
	for (unsigned char c : in) {
		val = (val<<8) + c;
		valb += 8;
		while (valb>=0) {
			out.push_back(base64chars[(val>>valb)&0x3F]);
			valb-=6;
		}
	}
	if (valb>-6) out.push_back(base64chars[((val<<8)>>(valb+8))&0x3F]);
	while (out.size()%4) out.push_back('=');
	return out;
}


std::string to_gjp(std::string password) {
	
	const char* key = "37526";
	const size_t keyLen = 5;
	size_t gjpCurrent = 0;
	for(auto& character : password) {
		character ^= key[gjpCurrent];
		gjpCurrent = (gjpCurrent + 1) % keyLen;
	}
	return base64_encode(password);
}


std::string seed2_xor(std::string seed2) {
	
	const char* key = "41274";
	const size_t keyLen = 5;
	size_t gjpCurrent = 0;
	for(auto& character : seed2) {
		character ^= key[gjpCurrent];
		gjpCurrent = (gjpCurrent + 1) % keyLen;
	}
	return seed2;
}

std::string getSeed2(const GJGameLevel& level)
{
	std::string seed2 {};
	size_t space = level._levelString.length() / 50;
	for(size_t i = 0; i < 50; i++)
		seed2 += level._levelString.at(space * i);
	
	seed2 += "xI25fpAapCQg";
	SHA1 checksum;
	checksum.update(seed2);
	seed2 = checksum.final();
	seed2 = seed2_xor(seed2);
	return base64_encode(seed2);
}




const char* get_node_name(ax::Node* node)
{
	return typeid(*node).name() + 6;
}

void logNames(ax::Node* n) {
	auto objects = n->getChildren();
	
	for(int i = 0; i < objects.size(); i++) {
		auto node = objects.at(i);
		AXLOG("i: %d | %s | count: %d", node->getTag(), get_node_name(node), node->getChildrenCount());
	}
	AXLOG("Finished");

}

void setLabelText(std::string_view text)
{
	ax::Node* stuff = ax::Director::getInstance()->getRunningScene()->getChildren().at(1);
	if(!stuff) return;
	
	//logNames(stuff);
	
	auto resultLabel = dynamic_cast<ax::Label*>(stuff->getChildByTag(1945));
	if(!resultLabel) return;
	
	resultLabel->setString(text);
}