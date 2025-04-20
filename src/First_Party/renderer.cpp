#include "renderer.h"

void Renderer::Render() {

	std::stable_sort(image_render_requests.begin(), image_render_requests.end(), [](const ImageRenderRequest& a, const ImageRenderRequest& b) {
		return a.sorting_order < b.sorting_order; 
		});

	std::stable_sort(ui_render_requests.begin(), ui_render_requests.end(), [](const UIRenderRequest& a, const UIRenderRequest& b) {
		return a.sorting_order < b.sorting_order;
		});

	SDL_RenderSetScale(renderer, camera.GetZoom(), camera.GetZoom());

	if (!image_render_requests.empty()) {
		for (ImageRenderRequest& request : image_render_requests)
			ImageDB::Draw(request);
		image_render_requests.clear();
	}

	SDL_RenderSetScale(renderer, 1.0f, 1.0f);

	if (!ui_render_requests.empty()) {
		for (UIRenderRequest& request : ui_render_requests)
			ImageDB::DrawUI(request);
		ui_render_requests.clear();
	}
	

	if (!text_render_requests.empty()) {
		for (TextRenderRequest& request : text_render_requests)
			TextDB::RequestText(request);
		text_render_requests.clear();
	}

	if (!pixel_render_requests.empty()) {
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		for (PixelRenderRequest& request : pixel_render_requests)
			RenderPixelRequest(request);
		pixel_render_requests.clear();
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	}

}

void Renderer::DrawText(const std::string text, const float x, const float y, const char* font,
	const float fontSize, const float r, const float g, const float b, const float a) {
	SDL_Color fontColor = { static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a) };

	std::string fontName(font);
	//TextRenderRequest request(text, (int)x, (int)y, font, (int)fontSize, fontColor);

	text_render_requests.emplace_back(text, (int)x, (int)y, fontName, (int)fontSize, fontColor);
}

void Renderer::DrawUI(std::string image, float x, float y) {
	ui_render_requests.emplace_back(image, static_cast<int>(x), static_cast<int>(y));
}

void Renderer::DrawUIEx(std::string image, float x, float y, float r, float g, float b, float a, float sorting_order) {
	SDL_Color color = { static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a) };
	ui_render_requests.emplace_back(image, color, static_cast<int>(x), static_cast<int>(y), static_cast<int>(sorting_order));
}

void Renderer::DrawImage(std::string image, float x, float y) {

	glm::vec2 final_rendering_position = glm::vec2(x, y) - CameraGetPosition();

	ImageData* imageData = ImageDB::GetImage(image);

	SDL_Texture* tex = imageData->texture;
	SDL_FRect tex_rect;
	tex_rect.w = imageData->width;
	tex_rect.h = imageData->height;

	//tex_rect.w *= glm::abs(request.scaleX);
	//tex_rect.h *= glm::abs(request.scaleY);

	SDL_FPoint pivot_point = { 0.5f * tex_rect.w, 0.5f * tex_rect.h };

	//glm::ivec2 cam_dimensions = Renderer::GetCameraDimensions();

	float zoom_factor = CameraGetZoom();
	float inverse_zoom_factor = CameraGetInverseZoom();

	tex_rect.x = final_rendering_position.x * ImageDB::pixels_per_meter + GameData::CAMERA_HALF_WIDTH * inverse_zoom_factor - pivot_point.x;
	tex_rect.y = final_rendering_position.y * ImageDB::pixels_per_meter + GameData::CAMERA_HALF_HEIGHT * inverse_zoom_factor - pivot_point.y;

	// If character is off-camera, don't render
	if (tex_rect.x + tex_rect.w < 0.0f ||
		tex_rect.y + tex_rect.h < 0.0f ||
		tex_rect.x > GameData::CAMERA_WIDTH * inverse_zoom_factor ||
		tex_rect.y > GameData::CAMERA_HEIGHT * inverse_zoom_factor)
		return;

	int flip_mode = SDL_FLIP_NONE;

	SDL_Color color = { 255, 255, 255, 255 };

	//image_render_requests.emplace_back(image, x, y);

	image_render_requests.emplace_back(tex, tex_rect, pivot_point, color, 0, flip_mode, 0);
}

void Renderer::DrawImageEx(std::string image, float x, float y, float rotation_degrees,
	float scale_x, float scale_y, float pivot_x, float pivot_y,
	float r, float g, float b, float a, float sorting_order) {

	glm::vec2 final_rendering_position = glm::vec2(x, y) - CameraGetPosition();

	ImageData* imageData = ImageDB::GetImage(image);

	SDL_Texture* tex = imageData->texture;
	SDL_FRect tex_rect;
	tex_rect.w = imageData->width * glm::abs(scale_x);
	tex_rect.h = imageData->height * glm::abs(scale_y);
	//Helper::SDL_QueryTexture(tex, &tex_rect.w, &tex_rect.h);

	//tex_rect.w *= glm::abs(request.scaleX);
	//tex_rect.h *= glm::abs(request.scaleY);

	SDL_FPoint pivot_point = { pivot_x * tex_rect.w, pivot_y * tex_rect.h };

	//glm::ivec2 cam_dimensions = Renderer::GetCameraDimensions();

	float zoom_factor = CameraGetZoom();
	float inverse_zoom_factor = CameraGetInverseZoom();

	tex_rect.x = final_rendering_position.x * ImageDB::pixels_per_meter + GameData::CAMERA_HALF_WIDTH * inverse_zoom_factor - pivot_point.x;
	tex_rect.y = final_rendering_position.y * ImageDB::pixels_per_meter + GameData::CAMERA_HALF_HEIGHT * inverse_zoom_factor - pivot_point.y;

	// If character is off-camera, don't render
	if (tex_rect.x + tex_rect.w < 0.0f ||
		tex_rect.y + tex_rect.h < 0.0f ||
		tex_rect.x > GameData::CAMERA_WIDTH * inverse_zoom_factor ||
		tex_rect.y > GameData::CAMERA_HEIGHT * inverse_zoom_factor)
		return;

	// Apply scale
	int flip_mode = SDL_FLIP_NONE;
	if (scale_x < 0)
		flip_mode |= SDL_FLIP_HORIZONTAL;
	if (scale_y < 0)
		flip_mode |= SDL_FLIP_VERTICAL;

	SDL_Color color = { static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a) };
	//image_render_requests.emplace_back(image, color, x, y, static_cast<int>(sorting_order), static_cast<int>(rotation_degrees), scale_x, scale_y, pivot_x, pivot_y);

	image_render_requests.emplace_back(tex, tex_rect, pivot_point, color, static_cast<int>(rotation_degrees), flip_mode, static_cast<int>(sorting_order));
}

void Renderer::DrawParticleEx(std::string image, float x, float y, float rotation_degrees,
	float scale_x, float scale_y, float pivot_x, float pivot_y,
	uint8_t r, uint8_t g, uint8_t b, uint8_t a, int sorting_order) {

	glm::vec2 final_rendering_position = glm::vec2(x, y) - CameraGetPosition();

	ImageData* imageData = ImageDB::GetImage(image);

	SDL_Texture* tex = imageData->texture;
	SDL_FRect tex_rect;
	tex_rect.w = imageData->width * glm::abs(scale_x);
	tex_rect.h = imageData->height * glm::abs(scale_y);

	//tex_rect.w *= glm::abs(scale_x);
	//tex_rect.h *= glm::abs(scale_y);

	SDL_FPoint pivot_point = { pivot_x * tex_rect.w, pivot_y * tex_rect.h };

	float zoom_factor = CameraGetZoom();
	float inverse_zoom_factor = CameraGetInverseZoom();

	tex_rect.x = final_rendering_position.x * ImageDB::pixels_per_meter + GameData::CAMERA_HALF_WIDTH * inverse_zoom_factor - pivot_point.x;
	tex_rect.y = final_rendering_position.y * ImageDB::pixels_per_meter + GameData::CAMERA_HALF_HEIGHT * inverse_zoom_factor - pivot_point.y;

	// If character is off-camera, don't render
	if (tex_rect.x + tex_rect.w < 0.0f ||
		tex_rect.y + tex_rect.h < 0.0f ||
		tex_rect.x > GameData::CAMERA_WIDTH * inverse_zoom_factor ||
		tex_rect.y > GameData::CAMERA_HEIGHT * inverse_zoom_factor)
		return;

	// Apply scale
	int flip_mode = SDL_FLIP_NONE;
	if (scale_x < 0)
		flip_mode |= SDL_FLIP_HORIZONTAL;
	if (scale_y < 0)
		flip_mode |= SDL_FLIP_VERTICAL;

	SDL_Color color = { r, g, b, a };

	image_render_requests.emplace_back(tex, tex_rect, pivot_point, color, static_cast<int>(rotation_degrees), flip_mode, sorting_order);

	//image_render_requests.emplace_back(image, color, x, y, sorting_order, static_cast<int>(rotation_degrees), scale_x, scale_y, pivot_x, pivot_y);
}

void Renderer::DrawPixel(float x, float y, float r, float g, float b, float a) {
	SDL_Color color = { static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a) };
	pixel_render_requests.emplace_back(static_cast<int>(x), static_cast<int>(y), color);
}

void Renderer::RenderPixelRequest(PixelRenderRequest& request) {
	SDL_SetRenderDrawColor(renderer, request.color.r, request.color.g, request.color.b, request.color.a);

	SDL_RenderDrawPoint(renderer, request.x, request.y);
}

void Renderer::CameraSetPosition(float x, float y) {
	camera.SetPosition(x, y);
}

glm::vec2 Renderer::CameraGetPosition() {
	return camera.camera_position;
}

float Renderer::CameraGetPositionX() {
	return camera.GetPositionX();
}

float Renderer::CameraGetPositionY() {
	return camera.GetPositionY();
}

void Renderer::CameraSetZoom(float zoom_factor) {
	camera.SetZoom(zoom_factor);
}

float Renderer::CameraGetZoom() {
	return camera.GetZoom();
}

float Renderer::CameraGetInverseZoom() {
	return camera.GetInverseZoom();
}

glm::ivec2 Renderer::GetCameraDimensions() {
	return glm::ivec2(GameData::CAMERA_WIDTH, GameData::CAMERA_HEIGHT);
}

/*
// HOMEWORK 6 Gizmo

void Renderer::GizmoDrawColliders(Scene* currentScene) {
	SDL_RenderSetScale(renderer, zoomFactor, zoomFactor);

	for (Actor* actor : currentScene->actorsToRender) {
		SDL_FRect rect;

		// Draw this actor's collider
		if (actor->hasCollider) {

			
			//cameraMiddle.x +
			//(GameData::CELL_SIZE * actor->transform_position.x) - 
			//(pivotPoint.x * absScale.x) - cameraPos.x,
			
			// Baseline position relative to the camera
			rect.x = actor->transform_position.x - (actor->box_collider_width / 2.0f);
			rect.y = actor->transform_position.y - (actor->box_collider_height / 2.0f);

			// Offset by camera offset values
			rect.x = (rect.x) * GameData::CELL_SIZE - cameraPos.x;
			rect.y = (rect.y) * GameData::CELL_SIZE - cameraPos.y;

			// Centers the actors position on the center of the camera
			rect.x += cameraMiddle.x;
			rect.y += cameraMiddle.y;

			rect.w = actor->box_collider_width * GameData::CELL_SIZE;
			rect.h = actor->box_collider_height * GameData::CELL_SIZE;

			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			SDL_RenderDrawRectF(renderer, &rect);
			SDL_SetRenderDrawColor(renderer, clear_color_r, clear_color_g, clear_color_b, 255);
		}
	}

	SDL_RenderSetScale(renderer, 1.0f, 1.0f);
}
*/
