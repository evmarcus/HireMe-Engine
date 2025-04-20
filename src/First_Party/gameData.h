#pragma once
#ifndef GAMEDATA_H
#define GAMEDATA_H
#include "utility.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include <unordered_map>
#include <string>

class GameData {
public:
	// Define an unordered_map
	static inline std::unordered_map<std::string, std::string> messages;

    // Access with hasMember()
	static inline rapidjson::Document gameConfig;
    static inline rapidjson::Document renderingConfig;

    static inline bool renderingConfigExists = false;

    static inline int CAMERA_WIDTH = 13;
    static inline int CAMERA_HEIGHT = 9;
    static inline int CAMERA_HALF_WIDTH = 6;
    static inline int CAMERA_HALF_HEIGHT = 4;

    static inline float CELL_SIZE = 100.0f;

    static inline int UUID = 0;
    static inline int CallsToAddComponent = 0;

    // Public method to access the single instance of the class
    static GameData& getInstance() {
        static GameData instance; // Guaranteed to be created only once
        return instance;
    }

    static void ReadRenderingConfig() {
        renderingConfigExists = true;

        SetRenderingDefaults();

        if (renderingConfig.HasMember("x_resolution")) {
            CAMERA_WIDTH = renderingConfig["x_resolution"].GetInt();
        }
        if (renderingConfig.HasMember("y_resolution")) {
            CAMERA_HEIGHT = renderingConfig["y_resolution"].GetInt();
        }

        CAMERA_HALF_WIDTH = static_cast<int>(CAMERA_WIDTH / 2.0f);
        CAMERA_HALF_HEIGHT = static_cast<int>(CAMERA_HEIGHT / 2.0f);
    }

    static void SetRenderingDefaults() {
        CAMERA_WIDTH = 640;
        CAMERA_HALF_WIDTH = static_cast<int>(CAMERA_WIDTH / 2.0f);
        CAMERA_HEIGHT = 360;
        CAMERA_HALF_HEIGHT = static_cast<int>(CAMERA_HEIGHT / 2.0f);
    }

    static int GetUUID() {
        return UUID++;
    }

    static std::string GetRuntimeComponentKey() {
        std::string key = "r" + std::to_string(CallsToAddComponent++);
        return key;
    }

    static int GetCameraWidth() {
        return CAMERA_WIDTH;
    }

    static int GetCameraHeight() {
        return CAMERA_HEIGHT;
    }

private:

    // Private constructor and destructor to prevent multiple instances
    GameData() { SetRenderingDefaults(); }
    ~GameData() = default;

    // Delete copy constructor and assignment operator to prevent copying
    GameData(const GameData&) = delete;
    GameData& operator=(const GameData&) = delete;
};

#endif