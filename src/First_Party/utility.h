#pragma once
#ifndef UTILITY_H
#define UTILITY_H
#include <iostream>
#include <limits>
#include "glm/glm.hpp"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "AudioHelper.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "box2d/box2d.h"

class TextRenderRequest {
public:
	std::string text;
	std::string font;
	SDL_Color color;
	int size;
	int x;
	int y;

	TextRenderRequest() :
		text(""), x(0), y(0), font(""), size(0), color({ 255, 255, 255, 255 }) {}

	TextRenderRequest(const std::string text, int x, int y, const std::string fontName, int size, SDL_Color& color) :
		text(text), x(x), y(y), font(fontName), size(size), color(color) {}
};

class OldImageRenderRequest {
public:
	std::string imageName;
	SDL_Color color;
	float x;
	float y;
	int sortingOrder;
	int rotation;
	float scaleX;
	float scaleY;
	float pivotX;
	float pivotY;

	OldImageRenderRequest(std::string image, float x, float y) :
		imageName(image), color({255, 255, 255, 255}), x(x), y(y), sortingOrder(0),
		rotation(0), scaleX(1.0f), scaleY(1.0f), pivotX(0.5f), pivotY(0.5f) {
	}

	OldImageRenderRequest(std::string image, SDL_Color color, float x, float y, int sortingOrder, 
		int rotation, float scaleX, float scaleY, float pivotX, float pivotY) : 
		imageName(image), color(color), x(x), y(y), sortingOrder(sortingOrder), 
		rotation(rotation), scaleX(scaleX), scaleY(scaleY), pivotX(pivotX), pivotY(pivotY) {}

};

class UIRenderRequest {
public:
	std::string imageName;
	SDL_Color color;
	float x;
	float y;
	int sorting_order;

	UIRenderRequest(std::string image, float x, float y) :
		imageName(image), color({ 255, 255, 255, 255 }), x(x), y(y), sorting_order(0) {
	}

	UIRenderRequest(std::string image, SDL_Color color, float x, float y, int sortingOrder) :
		imageName(image), color(color), x(x), y(y), sorting_order(sortingOrder) {
	}

};

class ImageRenderRequest {
public:
	SDL_Texture* tex;
	SDL_FRect tex_rect;
	SDL_FPoint pivot_point;
	SDL_Color color;
	int rotation;
	int flip_mode;
	int sorting_order;

	ImageRenderRequest(SDL_Texture* _tex, SDL_FRect _tex_rect, SDL_FPoint _pivot_point, SDL_Color _color, int _rotation, int _flip_mode, int _sorting_order) :
		tex(_tex), tex_rect(_tex_rect), pivot_point(_pivot_point), color(_color), rotation(_rotation), flip_mode(_flip_mode), sorting_order(_sorting_order) {
	}

};

class PixelRenderRequest {
public:
	SDL_Color color;
	int x;
	int y;

	PixelRenderRequest(int x, int y, SDL_Color color) : x(x), y(y), color(color) {}

};

// TODO: Copy the rest of the render structures from slide 27 on in discussion 8 slides

class EngineUtils {
public:

	static void ReadJsonFile(const std::string& path, rapidjson::Document& out_document)
	{
		FILE* file_pointer = nullptr;
#ifdef _WIN32
		fopen_s(&file_pointer, path.c_str(), "rb");
#else
		file_pointer = fopen(path.c_str(), "rb");
#endif
		char buffer[65536];
		rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
		out_document.ParseStream(stream);
		std::fclose(file_pointer);

		if (out_document.HasParseError()) {
			rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
			std::cout << "error parsing json at [" << path << "]" << std::endl;
			exit(0);
		}
	}

	// Pass in the json file to read from if it exists, and the key, and this function
	// will give you the string associated if it exists, or "" if it doesn't
	static std::string GetStringByKey(rapidjson::Document& document, std::string key) {
		//whateverString = myDocument["game_whatever_message"].GetString()
		const char* key_str = key.c_str();
		if (document.HasMember(key_str)) {
			std::string cppString(document[key_str].GetString());
			return cppString;
		}

		return "";
	}

	// Pass in the json file to read from if it exists, and the key, and this function
	// will give you the string associated if it exists, or infinity if it doesn't
	static int GetIntByKey(rapidjson::Document& document, std::string key) {
		//whateverString = myDocument["game_whatever_message"].GetString()
		const char* key_str = key.c_str();
		if (document.HasMember(key_str)) {
			int value(document[key_str].GetInt());
			return value;
		}

		return std::numeric_limits<int>::infinity();
	}

	static uint64_t CreateCompositeKey(int x, int y) {
		//cast to ensure the ints become exactly 32 bits in size
		uint32_t ux = static_cast<uint32_t>(x);
		uint32_t uy = static_cast<uint32_t>(y);

		//place x into right 32 bits
		uint64_t result = static_cast<uint64_t>(ux);

		//move x to left 32 bits
		result = result << 32;

		//place y into right 32 bits
		result = result | static_cast<uint64_t>(uy);

		return result;
	}

	static void ReportError(const std::string& actor_name, const luabridge::LuaException& e);
};

#endif