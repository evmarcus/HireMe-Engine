#include "game.h"
#include "gameData.h"
#include <filesystem>

void CheckDependencies();
void ReadConfigs();

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {

	CheckDependencies();
	ReadConfigs();

	Game::Init();

	Game::RunGame();
	
	return 0;
}

void CheckDependencies() {
	// Get the path of the current executable
	fs::path exePath = fs::current_path();

	// Append the "resources" directory to the path
	fs::path resourcesDir = exePath / "resources";

	// Check if the "resources" directory exists
	if (!fs::exists(resourcesDir) || !fs::is_directory(resourcesDir)) {
		std::cout << "error: resources/ missing";
		exit(0);
	}

	fs::path gameConfigDir = resourcesDir / "game.config";

	if (!fs::exists(gameConfigDir)) {
		std::cout << "error: resources/game.config missing";
		exit(0);
	}
}

void ReadConfigs() {
	GameData& data = GameData::getInstance();

	// Get the path of the current executable
	fs::path exePath = fs::current_path();

	// Append the "resources" directory to the path
	fs::path gameConfigPath = exePath / "resources" / "game.config";

	std::string path = gameConfigPath.string();
	EngineUtils::ReadJsonFile(path, data.gameConfig);

	// if rendering.config exists, read it in
	fs::path renderingConfigDir = exePath / "resources" / "rendering.config";

	if (fs::exists(renderingConfigDir)) {
		EngineUtils::ReadJsonFile(renderingConfigDir.string(), data.renderingConfig);
		data.ReadRenderingConfig();
	}
	else {
		data.SetRenderingDefaults();
	}
}
