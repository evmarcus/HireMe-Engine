#pragma once
#ifndef GAME_H
#define GAME_H
#include "renderer.h"
#include "utility.h"
#include "gameData.h"
#include "SceneDB.h"
#include "ImageDB.h"
#include "AudioDB.h" // MIGHT CAUSE CIRCULAR DEPENDENCY
#include "Input.h"
#include <algorithm> 
#include <thread>
#include <chrono>

using Clock = std::chrono::steady_clock;

class Game
{
public:
	static inline Renderer* renderer = nullptr;
	
	static inline Scene* currentScene = nullptr;

	static inline bool proceed_to_next_scene = false;

	static inline std::string next_scene = "";

	static inline const Clock::time_point startTime = Clock::now();

	//glm::fvec2 cameraPos = glm::fvec2(0.0f, 0.0f);

	//int gameState = 0; // 0 == intro, 1 == scene, 2 == ending

	//bool gameOver;

	static inline bool input_quit;

	static Game& getInstance() {
		static Game instance; // Guaranteed to be created only once
		return instance;
	}

	static void Init();

	static void RunGame();

	static void ProcessInput();

	static void Update();

	static void LateUpdate();

	static luabridge::LuaRef Find(const std::string& name);

	static luabridge::LuaRef FindAll(const std::string& name);

	static luabridge::LuaRef Instantiate(const std::string& actor_template_name);

	static void Destroy(Actor* actor);

	static void Quit();

	static void Sleep(int milliseconds);

	static float GetTime();

	static void OpenURL(const std::string& url);

	static void QueueSceneLoad(std::string sceneName);

	static void LoadScene(std::string sceneName);

	static std::string GetCurrentSceneName();

	static void DontDestroyOnLoad(Actor* actor);

	//bool GameStateIsIntro() { return gameState == 0; }
	//bool GameStateIsScene() { return gameState == 1; }
	//bool GameStateIsEnding() { return gameState == 2; }
	// Read in all the input events of the SDL Window, 
	// return true if quit command detected
	//void RenderIntro();
	// move the intro image and text indices to their next spot
	//void ProcessIntro();
	//void RenderScene();
	// And check for new scene loads
	//std::string UpdateHealthAndScore(const std::string& dialogue, Actor* actor);
	//void SetStateToEnding();
	//void RenderEnding();
	//bool GameOver();
	//void RenderStart();
	//void RenderGameOver();
	//void UpdateActors();
	//glm::fvec2 GetDirectionOfInput();
	//void Quit();
private:
	// Private constructor and destructor to prevent multiple instances
	Game() {}
	~Game() = default;

	// Delete copy constructor and assignment operator to prevent copying
	Game(const Game&) = delete;
	Game& operator=(const Game&) = delete;
};

#endif