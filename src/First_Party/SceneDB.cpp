#include "SceneDB.h"

Scene* SceneDB::LoadScene(std::string sceneName) {

	// If the scene already exists, retrieve it and return
	/*if (scenes.find(sceneName) != scenes.end()) {
		Scene* scene = new Scene(*scenes[sceneName], sceneName);
		return scene;
	}*/

	// Get the path of the current executable
	//fs::path exePath = fs::current_path();

	fs::path sceneDir = fs::current_path() / "resources" / "scenes" / (sceneName + ".scene");

	if (!fs::exists(sceneDir)) {
		std::cout << "error: scene " << sceneName << " is missing";
		exit(0);
	}

	rapidjson::Document sceneDoc;
	EngineUtils::ReadJsonFile(sceneDir.string(), sceneDoc);

	Scene* scene = new Scene(sceneDoc, sceneName);

	//Scene* copyForCache = new Scene(*scene, sceneName);

	// Store a copy of the level's initial state in case we ever come back
	//scenes[sceneName] = scene;

	return scene; 
}