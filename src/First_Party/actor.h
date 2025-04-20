#pragma once
#ifndef ACTOR_H
#define ACTOR_H

#include "utility.h"
#include "ImageDB.h"
#include "AudioDB.h"
#include "ComponentManager.h"
#include <optional>
#include <map>
#include <cmath>

class Collision;

class Actor
{
public:
	std::string name;
	int id;
	bool destroyed;
	bool dontDestroyOnLoad;
	bool started;
	std::map<std::string, Component*> components;
	std::map<std::string, Component*> components_requiring_onupdate;
	std::map<std::string, Component*> components_requiring_onlateupdate;
	std::unordered_map<std::string, std::vector<Component*>> gettableComponents;
	std::queue<Component*> added_components;
	std::queue<Component*> removed_components;

	Actor(const rapidjson::Value& actor, int id) : 
		name(""), id(id), destroyed(false), dontDestroyOnLoad(false), started(false) {

		InitializeActor(actor);
	}

	Actor() : name(""), id(-1), destroyed(false), dontDestroyOnLoad(false), started(false) {}

	// Initialize actor from template, override with anything else passed in
	Actor(Actor* templatedActor, const rapidjson::Value& actor, int id) :
		name(templatedActor->name), id(id), destroyed(false), dontDestroyOnLoad(false)
	{
		for (auto it = templatedActor->components.begin(); it != templatedActor->components.end(); ++it) {
			Component* newComp = ComponentManager::GetTemplatedComponentInstance(it->second);
			components[it->first] = newComp;
			InjectConvenienceReferences(newComp);
			gettableComponents[newComp->type].emplace_back(newComp);
		}

		InitializeActor(actor);
	}

	// Initialize actor from template
	Actor(Actor* templatedActor, int id) :
		name(templatedActor->name), id(id), destroyed(false), dontDestroyOnLoad(false), started(false)
	{
		for (auto it = templatedActor->components.begin(); it != templatedActor->components.end(); ++it) {
			Component* newComp = ComponentManager::GetTemplatedComponentInstance(it->second);
			components[it->first] = newComp;
			InjectConvenienceReferences(newComp);
			gettableComponents[newComp->type].emplace_back(newComp);
		}

		if (!components.empty()) {
			RefreshAcceleratingStructures();
		}
	}

	void InitializeActor(const rapidjson::Value& actor);

	void QueueStartFunctions(std::queue<Component*>& functionQueue);

	void Start();

	void ProcessAddedComponents();

	void Update();

	void LateUpdate();

	// TODO: Write InjectConvenienceReferences per slide 26 of discussion 7
	void InjectConvenienceReferences(Component* component_ref);

	void RefreshAcceleratingStructures();

	//void ReportError(const std::string& actor_name, const luabridge::LuaException& e);

	std::string GetName();

	int GetID();

	// Obtains reference to a component via key.
	// (return nil if the key doesn’t exist)
	luabridge::LuaRef GetComponentByKey(const std::string& key);

	// Obtains reference to component via type.
	// If multiple components exist of a type, return the first(sorted by component key).
	// (return nil if no components of the type exist)
	luabridge::LuaRef GetComponent(const std::string& type_name);

	// Obtains reference to all components of type.
	// (return in the form of an indexed table that may be iterated through with ipairs()).
	// The components should be returned in sorted order(by their key)
	// Hint: remember that Lua tables index starting at 1, not 0
	// (return an empty table if no components of the desired type exist)
	luabridge::LuaRef GetComponents(const std::string& type_name);

	// Add component to actor and return reference to it.
	// The new component should begin executing lifecycle functions on the next frame.
	// The component’s “key”(which remember, determines execution order) has a formula–
	// “rn” where “r” stands for runtime - added and “n” is the number of times AddComponent(type_name)
	// has been called in the entire program(“n” is a global integer counter, not one local to the actor).
	luabridge::LuaRef AddComponent(const std::string& type_name);

	void RemoveComponent(const luabridge::LuaRef& component_ref);

	void ProcessRemovedComponents();

	// Mark the actor and all of its components for destruction
	void Destroy();

	// Remove the actor and all of its components from memory
	void OnDestroy();

	void OnCollisionEnter(Collision* collision);

	void OnCollisionExit(Collision* collision);

	void OnTriggerEnter(Collision* collision);

	void OnTriggerExit(Collision* collision);
};

#endif