#pragma once
#ifndef TEXTDB_H
#define TEXTDB_H

#include <filesystem>
#include <unordered_map>
#include "utility.h"
#include "SDL_ttf/SDL_ttf.h"

namespace fs = std::filesystem;

// Custom hash function for SDL_Color
struct SDL_ColorHash {
    std::size_t operator()(const SDL_Color& color) const {
        return (color.r << 24) | (color.g << 16) | (color.b << 8) | color.a;
    }
};

// Custom equality function for SDL_Color (needed for unordered_map)
struct SDL_ColorEqual {
    bool operator()(const SDL_Color& lhs, const SDL_Color& rhs) const {
        return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
    }
};

class TextDB
{
public:
    static inline std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> fonts;

    // text textures accessible by [text content][font][color]
    static inline std::unordered_map<std::string, 
        std::unordered_map<TTF_Font*, 
        std::unordered_map<SDL_Color, SDL_Texture*, SDL_ColorHash, SDL_ColorEqual>>> text;

    // Public method to access the single instance of the class
    static TextDB& getInstance() {
        static TextDB instance; // Guaranteed to be created only once
        return instance;
    }

    static void Init() {
        TTF_Init();
    }

    static TTF_Font* GetFont(const std::string& fontName, const int fontSize);

    static SDL_Texture* GetText(TTF_Font* font, const std::string& text_content, SDL_Color fontColor);

    static void DrawText(const std::string& text_content, int x, int y,
        const std::string& fontName, const int fontSize, SDL_Color fontColor);

    static void RequestText(TextRenderRequest& request);

private:

    // Private constructor and destructor to prevent multiple instances
    TextDB() {}
    ~TextDB() = default;

    // Delete copy constructor and assignment operator to prevent copying
    TextDB(const TextDB&) = delete;
    TextDB& operator=(const TextDB&) = delete;

};

#endif