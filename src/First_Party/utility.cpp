#include "utility.h"

void EngineUtils::ReportError(const std::string& actor_name, const luabridge::LuaException& e) {
	std::string error_message = e.what();

	// Normalize file paths across platforms
	std::replace(error_message.begin(), error_message.end(), '\\', '/');

	// Find the "resources/" folder and strip everything before it
	std::string target = "resources/";
	size_t pos = error_message.find(target);
	if (pos != std::string::npos) {
		error_message = error_message.substr(pos); // Keep everything from "resources/" onwards
	}

	// Display (with color codes)
	std::cout << "\033[31m" << actor_name << " : " << error_message << "\033[0m\n";
}

/*
std::string EngineUtils::GetStringByKey(rapidjson::Document& document, std::string key) {
	//whateverString = myDocument["game_whatever_message"].GetString()
	const char* key_str = key.c_str();
	if (document.HasMember(key_str)) {
		std::string cppString(document[key_str].GetString());
		return cppString;
	}

	return "";
}
*/