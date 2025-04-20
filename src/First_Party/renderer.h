#pragma once
#ifndef RENDERER_H
#define RENDERER_H
#include "scene.h"
#include "utility.h"
#include "gameData.h"
#include "TextDB.h"
#include "ImageDB.h"
#include <iostream>
#include <sstream>
#include <set>

class Camera {
public:
	glm::fvec2 camera_position;
	float zoom_factor;
	float inverse_zoom_factor;

	Camera() : camera_position(glm::fvec2(0.0f, 0.0f)), zoom_factor(1.0f), inverse_zoom_factor(1.0f) {}

	void SetPosition(float x, float y) { camera_position = glm::fvec2(x, y); }

	float GetPositionX() { return camera_position.x; }

	float GetPositionY() { return camera_position.y; }

	void SetZoom(float _zoom_factor) { 
		zoom_factor = _zoom_factor; 
		inverse_zoom_factor = 1.0f / zoom_factor;
	}

	float GetZoom() { return zoom_factor; }

	float GetInverseZoom() { return inverse_zoom_factor; }
};

class Renderer
{
public:
	static inline SDL_Window* window = nullptr;
	static inline SDL_Renderer* renderer = nullptr;

	static inline Camera camera;

	static inline std::vector<TextRenderRequest> text_render_requests;
	static inline std::vector<UIRenderRequest> ui_render_requests;
	static inline std::vector<ImageRenderRequest> image_render_requests;
	static inline std::vector<PixelRenderRequest> pixel_render_requests;

	static inline int clear_color_r;
	static inline int clear_color_g;
	static inline int clear_color_b;

	TTF_Font* font = nullptr;

	static inline std::string fontName;


	Renderer() {
		GameData& data = GameData::getInstance();

		rapidjson::Document& gameConfig = GameData::gameConfig;
		rapidjson::Document& renderingConfig = GameData::renderingConfig;

		std::string gameTitle = EngineUtils::GetStringByKey(gameConfig, "game_title");

		window = Helper::SDL_CreateWindow(gameTitle.c_str(), 100, 100,
			data.CAMERA_WIDTH, data.CAMERA_HEIGHT, SDL_WINDOW_SHOWN);

		renderer = Helper::SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

		if (renderingConfig.IsNull()) {
			clear_color_r = 255;
			clear_color_g = 255;
			clear_color_b = 255;
		}
		else {
			if (renderingConfig.HasMember("clear_color_r")) clear_color_r = renderingConfig["clear_color_r"].GetInt();
			else clear_color_r = 255;

			if (renderingConfig.HasMember("clear_color_g")) clear_color_g = renderingConfig["clear_color_g"].GetInt();
			else clear_color_g = 255;

			if (renderingConfig.HasMember("clear_color_b")) clear_color_b = renderingConfig["clear_color_b"].GetInt();
			else clear_color_b = 255;
		}

		SDL_SetRenderDrawColor(renderer, clear_color_r, clear_color_g, clear_color_b, 255);

		if (data.renderingConfigExists) {
			
			if (renderingConfig.HasMember("zoom_factor")) {
				camera.SetZoom(renderingConfig["zoom_factor"].GetFloat());
			}
			else {
				camera.SetZoom(1.0f);
			}
		}

		ImageDB::SetRenderer(renderer);
	}

	// Public method to access the single instance of the class
	static Renderer& getInstance() {
		static Renderer instance; // Guaranteed to be created only once
		return instance;
	}

	static void Render();

	static void DrawText(const std::string text, const float x, const float y, const char* font,
		const float fontSize, const float r, const float g, const float b, const float a);

	static void DrawUI(std::string image, float x, float y);

	static void DrawUIEx(std::string image, float x, float y, float r, float g, float b, float a, float sorting_order);

	static void DrawImage(std::string image, float x, float y);

	static void DrawImageEx(std::string image, float x, float y, float rotation_degrees, 
		float scale_x, float scale_y, float pivot_x, float pivot_y, 
		float r, float g, float b, float a, float sorting_order);

	static void DrawParticleEx(std::string image, float x, float y, float rotation_degrees,
		float scale_x, float scale_y, float pivot_x, float pivot_y,
		uint8_t r, uint8_t g, uint8_t b, uint8_t a, int sorting_order);

	static void DrawPixel(float x, float y, float r, float g, float b, float a);

	static void RenderPixelRequest(PixelRenderRequest& request);

	static void CameraSetPosition(float x, float y);

	static glm::vec2 CameraGetPosition();

	static float CameraGetPositionX();
	
	static float CameraGetPositionY();

	static void CameraSetZoom(float zoom_factor);

	static float CameraGetZoom();

	static float CameraGetInverseZoom();

	static glm::ivec2 GetCameraDimensions();
};

#endif