#include "actor.h"
#include "gameData.h"

void Actor::InitializeActor(const rapidjson::Value& actor) {
	if (actor.HasMember("name")) {
		name = actor["name"].GetString();
	}
	if (actor.HasMember("components")) {
		//const auto& componentArray = actor["components"].GetArray();
		const rapidjson::Value& componentArray = actor["components"];

		for (auto it = componentArray.MemberBegin(); it != componentArray.MemberEnd(); ++it) {
			const rapidjson::Value& componentData = it->value;
			const std::string key = it->name.GetString();

			Component* newComp = nullptr;
			bool templated = false;

			// Don't override a potential templated component
			if (components.find(key) == components.end()) {
				std::string type = componentData["type"].GetString();

				newComp = ComponentManager::GetComponentInstance(type);
				components[key] = newComp;

				// Insert key self reference into component
				(*newComp->componentRef)["key"] = key.c_str();
			}
			else {
				newComp = components[key];
				templated = true;
			}

			// Apply overrides from other parts of the actor components to component instance
			for (auto fieldIt = componentData.MemberBegin(); fieldIt != componentData.MemberEnd(); ++fieldIt) {
				std::string componentType = fieldIt->name.GetString();
				if (componentType == "type")
					continue;

				// Handle different types dynamically
				if (fieldIt->value.IsString()) {
					(*newComp->componentRef)[componentType] = fieldIt->value.GetString();
				}
				else if (fieldIt->value.IsInt()) {
					(*newComp->componentRef)[componentType] = fieldIt->value.GetInt();
				}
				else if (fieldIt->value.IsFloat()) {
					(*newComp->componentRef)[componentType] = fieldIt->value.GetFloat();
				}
				else if (fieldIt->value.IsBool()) {
					// TODO: Switch checks like this to a Component bool for isCPPComponent
					//if (newComp->type == "Rigidbody") 
					if (newComp->isCPPComponent)
						(*newComp->componentRef)[componentType] = fieldIt->value.GetBool();
					else
						(*newComp->componentRef)[componentType] = (fieldIt->value.GetBool() ? "true" : "false");
				}
			}

			InjectConvenienceReferences(newComp);
			gettableComponents[newComp->type].emplace_back(newComp);
		}
	}

	if (!components.empty()) {
		RefreshAcceleratingStructures();
	}
}

void Actor::QueueStartFunctions(std::queue<Component*>& functionQueue) {
	for (auto it = components.begin(); it != components.end(); ++it) {
		if (it->second->hasStart) {
			//luabridge::LuaRef OnStartFunction = (*it->second->componentRef)["OnStart"];
			functionQueue.push(it->second);
		}
	}
}

void Actor::Start() {
	started = true;

	for (auto it = components.begin(); it != components.end(); ++it) {
		if (!it->second->hasStart)
			continue;

		if (it->second->IsEnabled()) {
			luabridge::LuaRef OnStartFunction = (*it->second->componentRef)["OnStart"];
			//luabridge::LuaRef enabled = (*it->second->componentRef)["enabled"];

			//std::cout << (*it->second->componentRef)["actor"];

			try {
				// Call lua OnUpdate() if enabled property is true
				//if (enabled.cast<bool>() == true)
				OnStartFunction(*it->second->componentRef);
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::ReportError(name, e);
			}
		}
	}
}

void Actor::ProcessAddedComponents() {
	if (added_components.empty())
		return;

	while (!added_components.empty()) {
		Component* newComp = added_components.front();

		if (newComp->IsEnabled() && newComp->hasStart) {
			luabridge::LuaRef OnStartFunction = (*newComp->componentRef)["OnStart"];
			//luabridge::LuaRef enabled = (*it->second->componentRef)["enabled"];

			try {
				// Call lua OnUpdate() if enabled property is true
				//if (enabled.cast<bool>() == true)
				OnStartFunction(*newComp->componentRef);
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::ReportError(name, e);
			}
		}

		components[(*newComp->componentRef)["key"].tostring()] = newComp;
		gettableComponents[newComp->type].emplace_back(newComp);

		added_components.pop();
	}

	RefreshAcceleratingStructures();
}

void Actor::Update() {
	for (auto it = components_requiring_onupdate.begin(); it != components_requiring_onupdate.end(); ++it) {
		if (it->second->IsEnabled()) {
			luabridge::LuaRef OnUpdateFunction = (*it->second->componentRef)["OnUpdate"];
			//luabridge::LuaRef enabled = (*it->second->componentRef)["enabled"];

			try {
				// Call lua OnUpdate() if enabled property is true
				//if (enabled.cast<bool>() == true)
					OnUpdateFunction(*it->second->componentRef);
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::ReportError(name, e);
			}
		}
	}
}

void Actor::LateUpdate() {
	for (auto it = components_requiring_onlateupdate.begin(); it != components_requiring_onlateupdate.end(); ++it) {
		if (it->second->IsEnabled()) {
			luabridge::LuaRef OnLateUpdateFunction = (*it->second->componentRef)["OnLateUpdate"];
			//luabridge::LuaRef enabled = (*it->second->componentRef)["enabled"];

			try {
				// Call lua OnUpdate() if enabled property is true
				//if (enabled.cast<bool>() == true)
				OnLateUpdateFunction(*it->second->componentRef);
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::ReportError(name, e);
			}
		}
	}
}

void Actor::InjectConvenienceReferences(Component* component_ref) {
	(*component_ref->componentRef)["actor"] = this;
	(*component_ref->componentRef)["enabled"] = true;
}

void Actor::RefreshAcceleratingStructures() {
	components_requiring_onupdate.clear();
	components_requiring_onlateupdate.clear();

	for (auto it = components.begin(); it != components.end(); ++it) {
		// Check each component for onStart, onUpdate, and onLateUpdate functions and do with them what I will
		//std::cout << it->second->type << ", ";
		Component* comp = it->second;
		std::string key = it->first;

		if (comp->hasUpdate)
			components_requiring_onupdate[key] = comp;
		if (comp->hasLateUpdate)
			components_requiring_onlateupdate[key] = comp;
	}
	//std::cout << "\n";
}

std::string Actor::GetName() {
	return name;
}

int Actor::GetID() {
	return id;
}

// Obtains reference to a component via key.
// (return nil if the key doesn’t exist)
luabridge::LuaRef Actor::GetComponentByKey(const std::string& key) {
	auto ref = components.find(key);
	if (ref != components.end() && ref->second->IsEnabled())
		return *ref->second->componentRef;
	else
		return luabridge::LuaRef(ComponentManager::GetLuaState()); // returns nil to lua
}

// Obtains reference to component via type.
// If multiple components exist of a type, return the first(sorted by component key).
// (return nil if no components of the type exist)
luabridge::LuaRef Actor::GetComponent(const std::string& type_name) {
	auto components_with_type = gettableComponents.find(type_name);

	if (components_with_type == gettableComponents.end())
		return luabridge::LuaRef(ComponentManager::GetLuaState()); // returns nil to lua if none found

	for (Component* component : components_with_type->second) {
		if (component->IsEnabled()) 
				return *component->componentRef;
	}

	/*
	for (auto it = components.begin(); it != components.end(); ++it) {
		if (it->second->IsEnabled()) {
			if (it->second->type == type_name)
				return *it->second->componentRef;
		}
	}*/

	return luabridge::LuaRef(ComponentManager::GetLuaState()); // returns nil to lua if none found
}

// Obtains reference to all components of type.
// (return in the form of an indexed table that may be iterated through with ipairs()).
// The components should be returned in sorted order(by their key)
// Hint: remember that Lua tables index starting at 1, not 0
// (return an empty table if no components of the desired type exist)
luabridge::LuaRef Actor::GetComponents(const std::string& type_name) {
	auto components_with_type = gettableComponents.find(type_name);

	if (components_with_type == gettableComponents.end())
		return luabridge::LuaRef(ComponentManager::GetLuaState()); // returns nil to lua if none found

	luabridge::LuaRef ref = luabridge::newTable(ComponentManager::GetLuaState());
	bool foundSomething = false;
	int index = 1; // Lua tables are 1-based

	for (Component* component : components_with_type->second) {
		if (component->IsEnabled()) {
			ref[index] = *component->componentRef; // Insert using numeric index
			foundSomething = true;
			index++;
		}
	}
	/*
	for (auto it = components.begin(); it != components.end(); ++it) {
		if (it->second->IsEnabled()) {
			if (it->second->type == type_name) {
				ref[index] = *it->second->componentRef; // Insert using numeric index
				foundSomething = true;
				index++;
			}
		}
	}*/

	if (foundSomething)
		return ref;

	return luabridge::LuaRef(ComponentManager::GetLuaState()); // returns nil to lua if none found
}

luabridge::LuaRef Actor::AddComponent(const std::string& type_name) {
	Component* newComp = ComponentManager::GetComponentInstance(type_name);
	std::string key = GameData::GetRuntimeComponentKey();

	// Insert key self reference into component
	(*newComp->componentRef)["key"] = key.c_str();
	InjectConvenienceReferences(newComp);

	added_components.push(newComp);

	return *newComp->componentRef;
}

void Actor::RemoveComponent(const luabridge::LuaRef& component_ref) {
	// TODO: MAKE SURE THAT THIS REGISTERS IN THE RIGIDBODY vv
	component_ref["enabled"] = false; 

	Component* comp = nullptr;

	for (auto it = components.begin(); it != components.end(); ++it) {
		if (*it->second->componentRef == component_ref) {
			comp = it->second;
			break;
		}
	}

	if (comp != nullptr)
		removed_components.push(comp);
}

void Actor::ProcessRemovedComponents() {
	if (removed_components.empty())
		return;

	while (!removed_components.empty()) {
		Component* newComp = removed_components.front();

		auto components_with_type = gettableComponents.find(newComp->type);

		if (components_with_type != gettableComponents.end()) {
			auto it = std::find(components_with_type->second.begin(), components_with_type->second.end(), newComp);
			if (it != components_with_type->second.end()) {
				components_with_type->second.erase(it);
			}
		}

		components.erase((*newComp->componentRef)["key"].tostring());

		// TODO DRY VIOLATION: Make a destroy component function 
		if (newComp->hasOnDestroy) {
			luabridge::LuaRef OnDestroyFunction = (*newComp->componentRef)["OnDestroy"];

			try {
				// Call lua OnDestroy() if it exists
				OnDestroyFunction(*newComp->componentRef);
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::ReportError(name, e);
			}
		}
		delete newComp;

		removed_components.pop();
	}

	RefreshAcceleratingStructures();
}

void Actor::Destroy() {
	destroyed = true;

	for (auto it = components.begin(); it != components.end(); ++it) {
		(*it->second->componentRef)["enabled"] = false;
	}
}

void Actor::OnDestroy() {
	for (auto it = components.begin(); it != components.end(); ++it) {
		if (it->second->hasOnDestroy) {
			luabridge::LuaRef OnDestroyFunction = (*it->second->componentRef)["OnDestroy"];

			try {
				// Call lua OnDestroy() if it exists
				OnDestroyFunction(*it->second->componentRef);
			}
			catch (const luabridge::LuaException& e) {
				EngineUtils::ReportError(name, e);
			}
		}
		delete it->second;
	}

	components.clear();
}

// TODO: Add accelerating structures for this
void Actor::OnCollisionEnter(Collision* collision) {
	if (destroyed) return;

	for (auto it = components.begin(); it != components.end(); ++it) {
		if (it->second->hasOnCollisionEnter) {
			if (it->second->IsEnabled()) {
				luabridge::LuaRef OnColEnterFunc = (*it->second->componentRef)["OnCollisionEnter"];

				try {
					// Call lua OnCollisionEnter() if enabled property is true
					OnColEnterFunc(*it->second->componentRef, collision);
				}
				catch (const luabridge::LuaException& e) {
					EngineUtils::ReportError(name, e);
				}
			}
		}
	}
}

void Actor::OnCollisionExit(Collision* collision) {
	if (destroyed) return;

	for (auto it = components.begin(); it != components.end(); ++it) {
		if (it->second->hasOnCollisionExit) {
			if (it->second->IsEnabled()) {
				luabridge::LuaRef OnColExitFunc = (*it->second->componentRef)["OnCollisionExit"];

				try {
					// Call lua OnCollisionExit() if enabled property is true
					OnColExitFunc(*it->second->componentRef, collision);
				}
				catch (const luabridge::LuaException& e) {
					EngineUtils::ReportError(name, e);
				}
			}
		}
	}
}

void Actor::OnTriggerEnter(Collision* collision) {
	if (destroyed) return;

	for (auto it = components.begin(); it != components.end(); ++it) {
		if (it->second->hasOnTriggerEnter) {
			if (it->second->IsEnabled()) {
				luabridge::LuaRef OnTrigEnterFunc = (*it->second->componentRef)["OnTriggerEnter"];

				try {
					// Call lua OnTriggerEnter() if enabled property is true
					OnTrigEnterFunc(*it->second->componentRef, collision);
				}
				catch (const luabridge::LuaException& e) {
					EngineUtils::ReportError(name, e);
				}
			}
		}
	}
}

void Actor::OnTriggerExit(Collision* collision) {
	if (destroyed) return;

	for (auto it = components.begin(); it != components.end(); ++it) {
		if (it->second->hasOnTriggerExit) {
			if (it->second->IsEnabled()) {
				luabridge::LuaRef OnTrigExitFunc = (*it->second->componentRef)["OnTriggerExit"];

				try {
					// Call lua OnTriggerExit() if enabled property is true
					OnTrigExitFunc(*it->second->componentRef, collision);
				}
				catch (const luabridge::LuaException& e) {
					EngineUtils::ReportError(name, e);
				}
			}
		}
	}
}
