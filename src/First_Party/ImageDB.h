#pragma once
#ifndef IMAGEDB_H
#define IMAGEDB_H

#include <filesystem>
#include <unordered_map>
#include "utility.h"
//#include "SDL_image.h"

namespace fs = std::filesystem;

struct ImageData {
    SDL_Texture* texture;
    float width, height;
};

class ImageDB {
public:
    static inline std::unordered_map<std::string, ImageData*> images;

    static inline SDL_Renderer* renderer = nullptr;

    static inline glm::ivec2 cam_dimensions = glm::ivec2(0, 0);

    static inline const int pixels_per_meter = 100;

    //IMG_LoadTexture()
    static void SetRenderer(SDL_Renderer* r) { renderer = r; }

    // Public method to access the single instance of the class
    static ImageDB& getInstance() {
        static ImageDB instance; // Guaranteed to be created only once
        return instance;
    }

    static void Init();

    static ImageData* GetImage(std::string imageName);

    static void Draw(ImageRenderRequest& request);

    static void DrawUI(UIRenderRequest& request);

    static void CreateDefaultParticleTextureWithName(const std::string& name);

private:
    

    // Private constructor and destructor to prevent multiple instances
    ImageDB() {}
    ~ImageDB() = default;

    // Delete copy constructor and assignment operator to prevent copying
    ImageDB(const ImageDB&) = delete;
    ImageDB& operator=(const ImageDB&) = delete;
};

#endif