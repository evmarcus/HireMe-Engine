#pragma once
#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include <filesystem>
#include <unordered_map>
#include "utility.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

namespace fs = std::filesystem;

class Component {
public:
    explicit Component(lua_State* lua_state, const std::string& filePath, const std::string& componentName);
    explicit Component(const Component& other, lua_State* lua_state);
    explicit Component();

    bool IsEnabled();

    std::shared_ptr<luabridge::LuaRef> componentRef;
    std::string type;

    bool isCPPComponent;

    bool hasStart;
    bool hasUpdate;
    bool hasLateUpdate;
    bool hasOnDestroy;
    bool hasOnCollisionEnter;
    bool hasOnCollisionExit;
    bool hasOnTriggerEnter;
    bool hasOnTriggerExit;
};

class ComponentManager
{
public:
    // Public method to access the single instance of the class
    static ComponentManager& getInstance() {
        static ComponentManager instance; // Guaranteed to be created only once
        return instance;
    }

    // Call before main loop begins
    static void Init();

    static void EstablishInheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table);

    // Returns an instance of a component by the type name of the component
    static Component* GetComponentInstance(std::string componentName);

    // Returns an instance of a component by the type name of the component
    static Component* GetTemplatedComponentInstance(Component* otherComponent);

    static Component* GetTemplatedCPPComponent(Component* otherComponent);

    static void CppDebugLog(const std::string message);

    static lua_State* GetLuaState();

    static Component* CreateRigidbody();

    static Component* CreateRigidbodyCopy(Component* otherComponent);

    static Component* CreateParticleSystem();

    static Component* CreateParticleSystemCopy(Component* otherComponent);

private:
    static inline lua_State* lua_state = nullptr;
    
    // TODO: maybe remove and just use the lua state for lookups
    static inline std::unordered_map<std::string, Component*> luaComponents;
    //static inline std::unordered_map<std::string, Component*> templates;

    // Private constructor and destructor to prevent multiple instances
    ComponentManager() {}
    ~ComponentManager() = default;

    // Delete copy constructor and assignment operator to prevent copying
    ComponentManager(const ComponentManager&) = delete;
    ComponentManager& operator=(const ComponentManager&) = delete;

};

#endif