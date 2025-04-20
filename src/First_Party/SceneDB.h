#pragma once
#ifndef SCENEDB_H
#define SCENEDB_H

#include <filesystem>
#include <unordered_map>
#include "utility.h"
#include "scene.h"

namespace fs = std::filesystem;

class SceneDB
{
public:
	static inline std::unordered_map<std::string, Scene*> scenes;

    // Public method to access the single instance of the class
    static SceneDB& getInstance() {
        static SceneDB instance; // Guaranteed to be created only once
        return instance;
    }

	static Scene* LoadScene(std::string sceneName);

private:

    // Private constructor and destructor to prevent multiple instances
    SceneDB() {}
    ~SceneDB() = default;

    // Delete copy constructor and assignment operator to prevent copying
    SceneDB(const SceneDB&) = delete;
    SceneDB& operator=(const SceneDB&) = delete;
};

#endif