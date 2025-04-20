#pragma once
#ifndef SCENE_H
#define SCENE_H
#include "utility.h"
#include "gameData.h"
#include "TemplateDB.h"
#include "actor.h"
#include <string>
#include <iostream>
#include <algorithm> // For std::sort
#include <unordered_map>
#include <limits>
#include <unordered_set>

/*struct Space
{
public:
	std::vector<Actor*> actors;
	char view;
	bool isBlocking;

	Space(char _view, bool _isBlocking)
		: view(_view), isBlocking(_isBlocking) {
	}

	Space() : view(' '), isBlocking(false) {}

	void AddActor(Actor* actor) {
		actors.push_back(actor);
		if (actor->blocking == true)
			isBlocking = true;
		SortActorVector();
		view = actors[actors.size() - 1]->view;
	}

	void RemoveActor(Actor* actor) {
		// Use std::remove to shift elements that don't match `actor` to the front.
		auto victim = std::remove(actors.begin(), actors.end(), actor);

		// Erase the "removed" part of the vector (from `it` to the end).
		actors.erase(victim, actors.end());

		isBlocking = false;
		view = ' ';

		// If no actors, no one is blocking
		if (actors.size() == 0)
			return;

		// Use an iterator to loop through the vector to see if this space is still blocking
		for (std::vector<Actor*>::iterator it = actors.begin(); it != actors.end(); ++it) {
			// Dereference the iterator to access the Actor*
			Actor* a = *it;
			if (a->blocking == true)
				isBlocking = true;
			view = a->view;
		}
	}

	void SortActorVector() {
		std::vector<int> actorIndices;
		std::vector<Actor*> actorVector;

		// Use an iterator to loop through the vector
		for (std::vector<Actor*>::iterator it = actors.begin(); it != actors.end(); ++it) {
			// Dereference the iterator to access the Actor*
			Actor* a = *it;
			actorIndices.push_back(GetActorIndex(a));
		}

		std::sort(actorIndices.begin(), actorIndices.end());

		for (std::vector<int>::iterator it = actorIndices.begin(); it != actorIndices.end(); ++it) {
			actorVector.push_back(&hardcoded_actors[*it]);
		}

		actors = actorVector;
	}

	int GetActorIndex(Actor* actor) {
		for (int i = 0; i < hardcoded_actors.size(); i++) {
			if (&hardcoded_actors[i] == actor)
				return i;
		}

		return -1;
	}

};*/

class Scene
{
public:
	std::string scene_name;

	std::vector<Actor*> actors;
	std::vector<Actor*> actors_to_add;
	std::unordered_map<std::string, std::vector<Actor*>> findActors;

	// Actors to destroy, by their index in actors array
	std::vector<int> actors_to_destroy;

	std::vector<Actor*> dont_destroy_on_load_actors;
	std::unordered_set<Actor*> DDOL_actors_in_scene;

	//std::vector<Actor*> actorsToUpdate;
	//std::vector<Actor*> actorsToLateUpdate;

	// map of names to vectors of Actor*
	//std::unordered_map<std::string, Actor*> actorsByName;

	std::queue<Component*> functionQueue;

	// Store the actors that have a view_image to avoid 
	// rendering a bunch of actors that don't show up
	std::vector<Actor*> actorsToRender;


	// Constructor from JSON file
	Scene(rapidjson::Document& sceneDoc, std::string sceneName);

	// Constructor from cached scene object
	Scene(const Scene& other, std::string sceneName);

	~Scene() {
		for (Actor* actor : actors) {
			if (!actor->dontDestroyOnLoad) {
				actor->Destroy();
				actor->OnDestroy();
				delete actor;
			}
		}
	}

	void ProcessDontDestroyOnLoad(Scene* oldScene);

	std::string GetSceneName();

	luabridge::LuaRef Find(const std::string& name);

	luabridge::LuaRef FindAll(const std::string& name);

	//Updates all actors with an OnUpdate function
	void UpdateActors();

	//Updates all actors with an OnLateUpdate function
	void LateUpdate();

	luabridge::LuaRef Instantiate(const std::string& actor_template_name);

	void Destroy(Actor* actor);

	void DontDestroyOnLoad(Actor* actor);

	void RemoveFromFindActors(Actor* actor);

	//std::vector<Actor*> GetSortedActorVector(std::vector<Actor*> actors);

	// Call this before rendering
	//void SortActorsByRenderOrder();


};

#endif