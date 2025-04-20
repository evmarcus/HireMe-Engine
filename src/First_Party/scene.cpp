#include "scene.h"

Scene::Scene(rapidjson::Document& sceneDoc, std::string sceneName) {
	//GameData& data = GameData::getInstance();
	//TemplateDB& templates = TemplateDB::getInstance();
	scene_name = sceneName;

	const auto& actorsArray = sceneDoc["actors"].GetArray();

	int numActors = actorsArray.Size();

	actors.reserve(numActors * 2);

	Actor* newActor = nullptr;

	for (const auto& actor : actorsArray) {
		// In the case the actor is templated from another, read in the template 
		// first and make sure it exists
		if (actor.HasMember("template") && actor["template"].IsString()) {
			Actor* templatedActor = TemplateDB::GetTemplate(actor["template"].GetString());
			// Use template copy constructor to make new actor
			newActor = new Actor(templatedActor, actor, GameData::GetUUID());
		}
		else {
			newActor = new Actor(actor, GameData::GetUUID());
		}

		//actors.emplace_back(newActor);
		actors_to_add.emplace_back(newActor);
		findActors[newActor->name].emplace_back(newActor);
	}
}

Scene::Scene(const Scene& other, std::string sceneName) {
	scene_name = sceneName;

	actors.reserve(other.actors_to_add.size() * 2);

	for (Actor* actor : other.actors_to_add) {
		Actor* newActor = new Actor(actor, GameData::GetUUID());
		//actors.emplace_back(newActor);
		actors_to_add.emplace_back(newActor);
		findActors[newActor->name].emplace_back(newActor);
	}
}

std::string Scene::GetSceneName() {
	return scene_name;
}

void Scene::UpdateActors() {
	// TODO OPT Use accelerating data structures to keep track of actors with an onUpdate
	// component, instead of looping through all actors. The difficulty comes in keeping 
	// these up to date when actors can add/remove components from themselves or others

	if (!actors_to_add.empty()) {
		// In case actors get added in start functions
		std::vector<Actor*> new_actors = actors_to_add;
		actors_to_add.clear();

		for (Actor* actor : new_actors) {
			actor->Start();
			actors.emplace_back(actor);
		}
	}

	for (Actor* actor : actors)
		actor->ProcessAddedComponents();

	for (Actor* actor : actors)
		actor->Update();

	LateUpdate();

	for (Actor* actor : actors)
		actor->ProcessRemovedComponents();

	if (!actors_to_destroy.empty()) {
		// Loop backwards to avoid messing up any indices
		for (int i = (int)actors_to_destroy.size() - 1; i >= 0; i--) {
			Actor* actor_to_destroy = actors[actors_to_destroy[i]];

			RemoveFromFindActors(actor_to_destroy);

			if (actor_to_destroy->dontDestroyOnLoad) {
				DDOL_actors_in_scene.erase(actor_to_destroy);
			}
			actor_to_destroy->OnDestroy();
			delete actor_to_destroy;
			actors.erase(actors.begin() + actors_to_destroy[i]);
		}

		actors_to_destroy.clear();
	}
}

void Scene::LateUpdate() {
	// TODO OPT Use accelerating data structures to keep track of actors with an onUpdate
	// component, instead of looping through all actors. The difficulty comes in keeping 
	// these up to date when actors can add/remove components from themselves or others

	for (Actor* actor : actors)
		actor->LateUpdate();
}

luabridge::LuaRef Scene::Find(const std::string & name) {
	if (findActors.find(name) != findActors.end()) {
		if (findActors[name].size() != 0) {
			for (Actor* actor : findActors[name]) {
				if (!actor->destroyed)
					return luabridge::LuaRef(ComponentManager::GetLuaState(), actor);
			}
		}
	}

	/*
	for (Actor* actor : actors) {
		if (actor->name == name) {
			if (!actor->destroyed) 
				return luabridge::LuaRef(ComponentManager::GetLuaState(), actor);
		}
	}

	if (!actors_to_add.empty()) {
		for (Actor* actor : actors_to_add) {
			if (actor->name == name)
				return luabridge::LuaRef(ComponentManager::GetLuaState(), actor);
		}
	}
	*/

	return luabridge::LuaRef(ComponentManager::GetLuaState()); // returns nil to lua if none found
}

luabridge::LuaRef Scene::FindAll(const std::string & name) {
	luabridge::LuaRef ref = luabridge::newTable(ComponentManager::GetLuaState());
	bool foundSomething = false;
	int index = 1; // Lua tables are 1-based

	if (findActors.find(name) != findActors.end()) {
		if (findActors[name].size() != 0) {
			for (Actor* actor : findActors[name]) {
				if (!actor->destroyed) {
					ref[index] = actor; // Insert using numeric index
					foundSomething = true;
					index++;
				}
			}
		}
	}

	/*
	for (Actor* actor : actors) {
		if (actor->name == name && !actor->destroyed) {
			ref[index] = actor; // Insert using numeric index
			foundSomething = true;
			index++;
		}
	}

	if (!actors_to_add.empty()) {
		for (Actor* actor : actors_to_add) {
			if (actor->name == name) {
				ref[index] = actor; // Insert using numeric index
				foundSomething = true;
				index++;
			}
		}
	}
	*/

	if (foundSomething)
		return ref;

	return luabridge::LuaRef(ComponentManager::GetLuaState()); // returns nil to lua if none found
}

luabridge::LuaRef Scene::Instantiate(const std::string& actor_template_name) {

	// Use template copy constructor to make new actor
	Actor* newActor = new Actor(TemplateDB::GetTemplate(actor_template_name), GameData::GetUUID());

	actors_to_add.emplace_back(newActor);
	findActors[newActor->name].emplace_back(newActor);

	return luabridge::LuaRef(ComponentManager::GetLuaState(), newActor);
}

void Scene::Destroy(Actor* actor) {
	if (actor->destroyed) return;

	actor->Destroy();

	// TODO: OPT There may be a way to speed this up instead of having to loop through
	// all actors for every destroy call
	for (int i = 0; i < actors.size(); i++) {
		if (actor->GetID() == actors[i]->GetID()) {
			actors_to_destroy.emplace_back(i);
			RemoveFromFindActors(actor);
			return;
		}
	}

	// If actor was not in actors, it must have been added this frame.
	// Thus, we can just remove it directly from the actors to add array safely.
	if (!actors_to_add.empty()) {
		for (int i = 0; i < actors_to_add.size(); i++) {
			if (actor->GetID() == actors_to_add[i]->GetID()) {
				actor->OnDestroy();
				actors_to_add.erase(actors_to_add.begin() + i);
				RemoveFromFindActors(actor);
				delete actor;
				return;
			}
		}
	}
}

void Scene::ProcessDontDestroyOnLoad(Scene* oldScene) {
	if (oldScene->dont_destroy_on_load_actors.empty())
		return;

	std::vector<Actor*> DDOLActors;
	DDOLActors.reserve(oldScene->dont_destroy_on_load_actors.size());

	for (Actor* actor : oldScene->dont_destroy_on_load_actors) {
		if (actor == nullptr)
			continue;

		DontDestroyOnLoad(actor);
		DDOLActors.emplace_back(actor);
		findActors[actor->name].insert(findActors[actor->name].begin(), actor);
	}

	// Insert DDOL Actors at beginning so they come up first in Find()
	actors.insert(actors.begin(), DDOLActors.begin(), DDOLActors.end());
}

void Scene::DontDestroyOnLoad(Actor* actor) {
	actor->dontDestroyOnLoad = true;

	// Only add to structures if the actor isn't already in them
	if (DDOL_actors_in_scene.find(actor) == DDOL_actors_in_scene.end()) {
		DDOL_actors_in_scene.insert(actor);
		dont_destroy_on_load_actors.push_back(actor);
	}
}

void Scene::RemoveFromFindActors(Actor* actor) {
	// Remove the actor from the accelerating structure
	if (findActors.find(actor->name) != findActors.end()) {
		if (findActors[actor->name].size() != 0) {
			findActors[actor->name].erase(
				std::remove(findActors[actor->name].begin(),
					findActors[actor->name].end(), actor), findActors[actor->name].end());
		}
	}
}
