#include "game.h"
#include "Rigidbody.h"
#include "EventBus.h"

void Game::Init() {
	renderer = &Renderer::getInstance();
	GameData& data = GameData::getInstance();
	ImageDB& imageDB = ImageDB::getInstance();
	ImageDB::Init();
	TextDB::Init();
	AudioDB::Init();
	Input::Init();
	ComponentManager::Init();

	if (data.gameConfig.HasMember("initial_scene")) {
		std::string name = data.gameConfig["initial_scene"].GetString();
		next_scene = name;
		LoadScene(name);
	}
	else
		currentScene = nullptr;
}

void Game::RunGame() {
	while (!input_quit) {
		ProcessInput(); 
		SDL_SetRenderDrawColor(Renderer::renderer, Renderer::clear_color_r, Renderer::clear_color_g, Renderer::clear_color_b, 255);
		SDL_RenderClear(Renderer::renderer);
		Update();
		EventBus::ProcessSubscriptions();
		Rigidbody::Step();
		Renderer::Render();
		Helper::SDL_RenderPresent(Renderer::renderer);
		if (proceed_to_next_scene)
			LoadScene(next_scene);
		Input::LateUpdate();
	}

	return;
}

void Game::ProcessInput() {
	SDL_Event event;
	while (Helper::SDL_PollEvent(&event)) {
		if (Input::ProcessEvent(event))
			input_quit = true;
	}
}

void Game::Update() {
	currentScene->UpdateActors();
}

void Game::LateUpdate() {
	currentScene->LateUpdate();
}

luabridge::LuaRef Game::Find(const std::string& name) {
	return currentScene->Find(name);
}

luabridge::LuaRef Game::FindAll(const std::string& name) {
	return currentScene->FindAll(name);
}

luabridge::LuaRef Game::Instantiate(const std::string& actor_template_name) {
	return currentScene->Instantiate(actor_template_name);
}

void Game::Destroy(Actor* actor) {
	return currentScene->Destroy(actor);
}

void Game::Quit() {
	exit(0);
}

void Game::Sleep(int milliseconds) {
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

float Game::GetTime() {
	auto now = Clock::now();
	std::chrono::duration<float> elapsed = now - startTime;
	return elapsed.count(); // returns seconds as a float
}

void Game::OpenURL(const std::string& url) {
	if (url.empty())
		return;

	std::string command;

#if defined(_WIN32) || defined(_WIN64)  // Windows
	command = "start " + url;
#elif defined(__APPLE__)  // macOS
	command = "open " + url;
#elif defined(__linux__)  // Linux
	command = "xdg-open " + url;
#endif

	int result = std::system(command.c_str());

	if (result != 0) {
		std::cout << "Failed to open URL: " << url << std::endl;
	}
}

void Game::QueueSceneLoad(std::string sceneName) {
	proceed_to_next_scene = true;
	next_scene = sceneName;
}

void Game::LoadScene(std::string sceneName) {
	proceed_to_next_scene = false;

	Scene* oldScene = currentScene;
	currentScene = SceneDB::LoadScene(sceneName);

	if (oldScene != nullptr) {
		currentScene->ProcessDontDestroyOnLoad(oldScene);

		delete oldScene;
	}
}

std::string Game::GetCurrentSceneName() {
	return currentScene->GetSceneName();
}

void Game::DontDestroyOnLoad(Actor* actor) {
	currentScene->DontDestroyOnLoad(actor);
}
