#include "ImageDB.h"
#include "renderer.h"

void ImageDB::Init() {
	cam_dimensions = Renderer::GetCameraDimensions();
}

ImageData* ImageDB::GetImage(std::string imageName) {
	// If the image already exists, retrieve it and return
	if (images.find(imageName) != images.end()) {
		return images[imageName];
	}

	// Get the path of the current executable
	fs::path exePath = fs::current_path();

	fs::path imageDir = exePath / "resources" / "images" / (imageName + ".png");

	if (!fs::exists(imageDir)) {
		std::cout << "error: missing image " << imageName;
		exit(0);
	}

	ImageData* data = new ImageData;
	data->texture = IMG_LoadTexture(renderer, imageDir.string().c_str());
	Helper::SDL_QueryTexture(data->texture, &data->width, &data->height);
	images[imageName] = data;

	return data;
}

void ImageDB::Draw(ImageRenderRequest& request) {

	/*glm::vec2 final_rendering_position = glm::vec2(request.x, request.y) - Renderer::CameraGetPosition();

	ImageData* image = GetImage(request.imageName);

	SDL_Texture* tex = image->texture;
	SDL_FRect tex_rect;
	tex_rect.w = image->width;
	tex_rect.h = image->height;
	//Helper::SDL_QueryTexture(tex, &tex_rect.w, &tex_rect.h);

	tex_rect.w *= glm::abs(request.scaleX);
	tex_rect.h *= glm::abs(request.scaleY);

	SDL_FPoint pivot_point = { request.pivotX * tex_rect.w, request.pivotY * tex_rect.h };

	//glm::ivec2 cam_dimensions = Renderer::GetCameraDimensions();

	float zoom_factor = Renderer::CameraGetZoom();

	tex_rect.x = final_rendering_position.x * pixels_per_meter + cam_dimensions.x * 0.5f * (1.0f / zoom_factor) - pivot_point.x;
	tex_rect.y = final_rendering_position.y * pixels_per_meter + cam_dimensions.y * 0.5f * (1.0f / zoom_factor) - pivot_point.y;

	// If character is off-camera, don't render
	/*if (tex_rect.x + tex_rect.w < 0.0f ||
		tex_rect.y + tex_rect.h < 0.0f ||
		tex_rect.x > GameData::CAMERA_WIDTH / zoom_factor ||
		tex_rect.y > GameData::CAMERA_HEIGHT / zoom_factor)
		return;

	// Apply scale
	int flip_mode = SDL_FLIP_NONE;
	if (request.scaleX < 0)
		flip_mode |= SDL_FLIP_HORIZONTAL;
	if (request.scaleY < 0)
		flip_mode |= SDL_FLIP_VERTICAL;*/

	// Apply tint / alpha to texture
	SDL_Color color = request.color;

	SDL_SetTextureColorMod(request.tex, color.r, color.g, color.b);
	SDL_SetTextureAlphaMod(request.tex, color.a);

	// Perform Draw
	Helper::SDL_RenderCopyEx(1, "", Renderer::renderer, request.tex, NULL, &request.tex_rect, request.rotation, &request.pivot_point, static_cast<SDL_RendererFlip>(request.flip_mode));

	//SDL_RenderSetScale(Renderer::renderer, zoom_factor, zoom_factor);

	// Remove tint / alpha from texture
	SDL_SetTextureColorMod(request.tex, 255, 255, 255);
	SDL_SetTextureAlphaMod(request.tex, 255);
}

void ImageDB::DrawUI(UIRenderRequest& request) {
	
	ImageData* image = GetImage(request.imageName);
	SDL_Texture* tex = image->texture;

	// Get texture width and height
	//float width, height;
	//Helper::SDL_QueryTexture(tex, &width, &height);

	// Define text position (e.g., x=300, y=250, width=200, height=50)
	SDL_FRect tex_rect = { request.x, request.y, image->width, image->height };

	// Apply tint / alpha to texture
	SDL_Color color = request.color;

	SDL_SetTextureColorMod(tex, color.r, color.g, color.b);
	SDL_SetTextureAlphaMod(tex, color.a);

	// Render text
	Helper::SDL_RenderCopy(Renderer::renderer, tex, NULL, &tex_rect);

	// Remove tint / alpha from texture
	SDL_SetTextureColorMod(tex, 255, 255, 255);
	SDL_SetTextureAlphaMod(tex, 255);
}

void ImageDB::CreateDefaultParticleTextureWithName(const std::string& name) {
	// Have we already cached this default texture?
	if (images.find(name) != images.end())
		return;
	
	// Create an SDL_Surface (a cpu-side texture) with no special flages, 8 width, 8 height, 32 bits of color depth (RGBA) and no masking.
	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 8, 8, 32, SDL_PIXELFORMAT_RGBA8888);

	// Ensure color set to white (255, 255, 255, 255)
	Uint32 white_color = SDL_MapRGBA(surface->format, 255, 255, 255, 255);
	SDL_FillRect(surface, NULL, white_color);

	// Create a gpu-side texture from the cpu-side surface now that we're done editing it.
	ImageData* data = new ImageData;
	data->texture = SDL_CreateTextureFromSurface(Renderer::renderer, surface);

	// Clean up the surface and cache this default texture for future use (we'll probably spawn many particles with it).
	SDL_FreeSurface(surface);
	Helper::SDL_QueryTexture(data->texture, &data->width, &data->height);
	images[name] = data;
}
