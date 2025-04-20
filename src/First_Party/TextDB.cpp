#include "TextDB.h"
#include "Helper.h"
#include "renderer.h"
#include <iostream>

TTF_Font* TextDB::GetFont(const std::string& fontName, const int fontSize) {
	// If the font already exists, retrieve it and return
	if (fonts.find(fontName) != fonts.end() && fonts[fontName].find(fontSize) != fonts[fontName].end()) {
		return fonts[fontName][fontSize];
	}

	// Get the path of the current executable
	fs::path exePath = fs::current_path();

	fs::path fontDir = exePath / "resources" / "fonts" / (fontName + ".ttf");

	if (!fs::exists(fontDir)) {
		std::cout << "error: font " << fontName << " missing";
		exit(0);
	}

	fonts[fontName][fontSize] = TTF_OpenFont(fontDir.string().c_str(), fontSize);

	return fonts[fontName][fontSize];
}

SDL_Texture* TextDB::GetText(TTF_Font* font, const std::string& text_content, SDL_Color fontColor) {

	// If no font is specified
	if (font == nullptr) {
		std::cout << "error: text render failed. No font configured";
		exit(0);
	}
	
	// If the text texture already exists, retrieve it and return
	if (text.find(text_content) != text.end())
		if (text[text_content].find(font) != text[text_content].end()) 
			if (text[text_content][font].find(fontColor) != text[text_content][font].end())
				return text[text_content][font][fontColor];

	SDL_Surface* textSurface = TTF_RenderText_Solid(font, text_content.c_str(), fontColor);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(Renderer::renderer, textSurface);
	SDL_FreeSurface(textSurface); // Free surface after conversion

	text[text_content][font][fontColor] = texture;

	return texture;
}

void TextDB::DrawText(const std::string& text_content, int x, int y,
	const std::string& fontName, const int fontSize, SDL_Color fontColor) {
	
	TTF_Font* font = GetFont(fontName, fontSize);

	SDL_Texture* text = GetText(font, text_content, fontColor);

	// Get texture width and height
	float width, height;
	Helper::SDL_QueryTexture(text, &width, &height);

	// Define text position (e.g., x=300, y=250, width=200, height=50)
	SDL_FRect textRect = { static_cast<float>(x), static_cast<float>(y), width, height };

	// Render text
	Helper::SDL_RenderCopy(Renderer::renderer, text, NULL, &textRect);
}

void TextDB::RequestText(TextRenderRequest& request) {
	DrawText(request.text, request.x, request.y, request.font, request.size, request.color);
}
