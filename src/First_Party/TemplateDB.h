#pragma once
#ifndef TEMPLATEDB_H
#define TEMPLATEDB_H

#include <filesystem>
#include <unordered_map>
#include "gameData.h"
#include "utility.h"
#include "actor.h"

namespace fs = std::filesystem;

class TemplateDB
{
public:
	static inline std::unordered_map<std::string, Actor*> templates;

    // Public method to access the single instance of the class
    static TemplateDB& getInstance() {
        static TemplateDB instance; // Guaranteed to be created only once
        return instance;
    }

	static Actor* GetTemplate(std::string templateName);
private:

    // Private constructor and destructor to prevent multiple instances
    TemplateDB() {}
    ~TemplateDB() = default;

    // Delete copy constructor and assignment operator to prevent copying
    TemplateDB(const TemplateDB&) = delete;
    TemplateDB& operator=(const TemplateDB&) = delete;
};

#endif