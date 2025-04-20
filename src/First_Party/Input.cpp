#include "Input.h"
#include "EventBus.h"
#include "ComponentManager.h"

void Input::Init() {
	InitControllers();

	// All inputs begin in the UP state
	for (int code = SDL_SCANCODE_UNKNOWN; code < SDL_NUM_SCANCODES; code++)
		keyboard_states[static_cast<SDL_Scancode>(code)] = INPUT_STATE_UP;
}

void Input::InitControllers() {
	if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
		std::cout << "SDL Joystick not initialized\n";
		return;
	}

	int num_controllers = SDL_NumJoysticks();

	/*if (num_controllers < 1) {
		std::cout << "No Joysticks detected\n";
		return;
	}

	std::cout << num_controllers << " joysticks connected\n";*/

	for (int i = 0; i < num_controllers; ++i) {
		AddController(i);
	}
}

void Input::AddController(int controller_num) {
	//if (SDL_IsGameController(controller_num)) {
		SDL_GameController* gc = SDL_GameControllerOpen(controller_num);
		if (gc) {
			SDL_Joystick* js = SDL_GameControllerGetJoystick(gc);
			SDL_JoystickID instanceID = SDL_JoystickInstanceID(js);
			unconnected_controller_ids.push(instanceID);
			Controller controller;
			controller.id = controller_num;
			controller.handle = gc;
			controller.instanceID = instanceID;

			controller.axes.resize(SDL_CONTROLLER_AXIS_MAX);
			//controller.buttons.resize(SDL_CONTROLLER_BUTTON_MAX);

			for (int code = SDL_CONTROLLER_BUTTON_A; code < SDL_CONTROLLER_BUTTON_MAX; code++)
				controller.button_states[static_cast<SDL_GameControllerButton>(code)] = INPUT_STATE_UP;

			controllers[instanceID] = controller;
		}
	//}
}

bool Input::ProcessEvent(const SDL_Event& event) {
	bool quit = false;

	switch (event.type) {
	case SDL_QUIT:
		quit = true;
		break;

	case SDL_MOUSEBUTTONDOWN:
		mouse_states[event.button.button] = INPUT_STATE_JUST_DOWN;
		just_became_down_mouse_buttons.push_back(event.button.button);
		break;

	case SDL_MOUSEBUTTONUP:
		mouse_states[event.button.button] = INPUT_STATE_JUST_UP;
		just_became_up_mouse_buttons.push_back(event.button.button);
		break;

	case SDL_KEYDOWN:
		keyboard_states[event.key.keysym.scancode] = INPUT_STATE_JUST_DOWN;
		just_became_down_scancodes.push_back(event.key.keysym.scancode);
		break;

	case SDL_KEYUP:
		keyboard_states[event.key.keysym.scancode] = INPUT_STATE_JUST_UP;
		just_became_up_scancodes.push_back(event.key.keysym.scancode);
		break;

	case SDL_MOUSEMOTION:
		mouse_position.x = static_cast<float>(event.motion.x);
		mouse_position.y = static_cast<float>(event.motion.y);
		break;

	case SDL_MOUSEWHEEL:
		mouse_scroll_this_frame = static_cast<float>(event.wheel.preciseY);
		break;

	case SDL_CONTROLLERAXISMOTION: {
		auto controller = controllers.find(event.caxis.which);
		if (controller == controllers.end()) break;
		//auto& controller = controllers[event.caxis.which];
		// Divide by 32767.0f to normalize
		int value = event.caxis.value;
		if (-8000 < value && value < 8000) value = 0; // Add dead zone
		controller->second.axes[event.caxis.axis] = value / 32767.0f;
		//std::cout << "axis: " << event.caxis.value << "\n";
		break;
	}
	case SDL_CONTROLLERBUTTONDOWN: {
		auto controller = controllers.find(event.cbutton.which);
		if (controller == controllers.end()) break;
		//auto& controller = controllers[event.cbutton.which];
		controller->second.button_states[event.cbutton.button] = INPUT_STATE_JUST_DOWN;
		controller->second.just_became_down_buttons.push_back(event.cbutton.button);
		just_became_down_controllers.push_back(event.cbutton.which);
		break;
	}
	case SDL_CONTROLLERBUTTONUP: {
		auto controller = controllers.find(event.cbutton.which);
		if (controller == controllers.end()) break;
		//auto& controller = controllers[event.cbutton.which];
		controller->second.button_states[event.cbutton.button] = INPUT_STATE_JUST_UP;
		controller->second.just_became_up_buttons.push_back(event.cbutton.button);
		just_became_up_controllers.push_back(event.cbutton.which);
		break;
	}
	case SDL_CONTROLLERDEVICEADDED:
		if (Helper::GetFrameNumber() != 0) {
			//if (disconnected_controller_ids.empty()) {
			AddController(event.cdevice.which);
			//}
			//else {
			//	int id = disconnected_controller_ids.front();
			//	AddController(num_controllers);
			//	disconnected_controller_ids.pop();
			//}
			OnControllerEvent("controller_connected", -1, event.cdevice.which);
			//num_controllers++;
		}
		break;
	case SDL_CONTROLLERDEVICEREMOVED: {
		SDL_JoystickID id = event.cdevice.which;
		auto it = controllers.find(id);
		if (it != controllers.end()) {
			OnControllerEvent("controller_disconnected", it->second.player_num, id);

			if (it->second.player_num != -1) {
				players.erase(it->second.player_num);
			}
			SDL_GameControllerClose(it->second.handle);
			controllers.erase(id);
			//disconnected_controller_ids.push(id);
			//num_controllers--;
		}
		break;
	}
	}

	return quit;
}

void Input::LateUpdate() {
	for (const SDL_Scancode& code : just_became_down_scancodes)
		keyboard_states[code] = INPUT_STATE_DOWN;
	just_became_down_scancodes.clear();

	for (const SDL_Scancode& code : just_became_up_scancodes)
		keyboard_states[code] = INPUT_STATE_UP;
	just_became_up_scancodes.clear();

	for (const Uint8& code : just_became_down_mouse_buttons)
		mouse_states[code] = INPUT_STATE_DOWN;
	just_became_down_mouse_buttons.clear();

	for (const Uint8& code : just_became_up_mouse_buttons)
		mouse_states[code] = INPUT_STATE_UP;
	just_became_up_mouse_buttons.clear();

	for (const SDL_JoystickID& code : just_became_down_controllers) {
		auto& controller = controllers[code];
		for (const Uint8& code : controller.just_became_down_buttons)
			controller.button_states[code] = INPUT_STATE_DOWN;
		controller.just_became_down_buttons.clear();
	}
	just_became_down_controllers.clear();

	for (const SDL_JoystickID& code : just_became_up_controllers) {
		auto& controller = controllers[code];
		for (const Uint8& code : controller.just_became_up_buttons)
			controller.button_states[code] = INPUT_STATE_UP;
		controller.just_became_up_buttons.clear();
	}
	just_became_up_controllers.clear();

	mouse_scroll_this_frame = 0.0f;
}

bool Input::GetKey(const std::string& keycode) {
	auto it = __keycode_to_scancode.find(keycode);
	if (it == __keycode_to_scancode.end())
		return false;
	const SDL_Scancode& scancode = it->second;
	return keyboard_states[scancode] == INPUT_STATE_DOWN || keyboard_states[scancode] == INPUT_STATE_JUST_DOWN;
}

bool Input::GetKeyDown(const std::string& keycode) {
	auto it = __keycode_to_scancode.find(keycode);
	if (it == __keycode_to_scancode.end())
		return false;
	const SDL_Scancode& scancode = it->second;
	return keyboard_states[scancode] == INPUT_STATE_JUST_DOWN;
}

bool Input::GetKeyUp(const std::string& keycode) {
	auto it = __keycode_to_scancode.find(keycode);
	if (it == __keycode_to_scancode.end())
		return false;
	const SDL_Scancode& scancode = it->second;
	return keyboard_states[scancode] == INPUT_STATE_JUST_UP;
}

glm::vec2 Input::GetMousePosition() {
	return mouse_position;
}

bool Input::GetMouseButton(Uint8 button_num) {
	if (mouse_states.find(button_num) != mouse_states.end())
		return mouse_states[button_num] == INPUT_STATE_DOWN || mouse_states[button_num] == INPUT_STATE_JUST_DOWN;
	mouse_states[button_num] = INPUT_STATE_UP;
	return false;
}

bool Input::GetMouseButtonDown(Uint8 button_num) {
	if (mouse_states.find(button_num) != mouse_states.end())
		return mouse_states[button_num] == INPUT_STATE_JUST_DOWN;
	mouse_states[button_num] = INPUT_STATE_UP;
	return false;
}

bool Input::GetMouseButtonUp(Uint8 button_num) {
	if (mouse_states.find(button_num) != mouse_states.end())
		return mouse_states[button_num] == INPUT_STATE_JUST_UP;
	mouse_states[button_num] = INPUT_STATE_UP;
	return false;
}

float Input::GetMouseScrollDelta() {
	return mouse_scroll_this_frame;
}

void Input::HideCursor() {
	SDL_ShowCursor(SDL_DISABLE);
}

void Input::ShowCursor() {
	SDL_ShowCursor(SDL_ENABLE);
}

bool Input::ConnectPlayerToController(int player_num) {
	if (unconnected_controller_ids.empty() || SDL_NumJoysticks() == 0)
		return false;
	
	auto player_it = players.find(player_num);

	if (player_it == players.end()) {
		int c_id = unconnected_controller_ids.front();
		players[player_num] = c_id;
		controllers[c_id].player_num = player_num;
		unconnected_controller_ids.pop();
        return true;
	}
    return false;
}

void Input::DisconnectPlayerFromController(int player_num) {
	auto player_it = players.find(player_num);

	if (player_it == players.end())
		return;
	else {
		controllers[player_it->second].player_num = -1;
		unconnected_controller_ids.push(player_it->second);
		players.erase(player_num);
	}
}

int Input::GetControllerIDByPlayerNum(int player_num) {
	auto player_it = players.find(player_num);

	if (player_it == players.end())
		return -1;
	else
		return controllers[player_it->second].id;
}

bool Input::ControllerConnected(int player_num) {
	if (players.find(player_num) != players.end())
		return true;
	return false;
}

int Input::GetNumberOfConnectedControllers() {
	return SDL_NumJoysticks();
}

bool Input::GetControllerButton(int player_num, const std::string& buttoncode) {
	auto player_it = players.find(player_num);

	if (player_it == players.end())
		return false;

	auto it = __buttonname_to_gamecontrollerbutton.find(buttoncode);
	if (it == __buttonname_to_gamecontrollerbutton.end())
		return false;

	Uint8 button = it->second;

	auto controller = controllers.find(player_it->second);
	if (controller == controllers.end()) return false;
	return controller->second.button_states[button] == INPUT_STATE_DOWN || 
		controller->second.button_states[button] == INPUT_STATE_JUST_DOWN;
}

bool Input::GetControllerButtonDown(int player_num, const std::string& buttoncode) {
	auto player_it = players.find(player_num);

	if (player_it == players.end())
		return false;

	auto it = __buttonname_to_gamecontrollerbutton.find(buttoncode);
	if (it == __buttonname_to_gamecontrollerbutton.end())
		return false;

	Uint8 button = it->second;

	auto controller = controllers.find(player_it->second);
	if (controller == controllers.end()) return false;
	return controller->second.button_states[button] == INPUT_STATE_JUST_DOWN;
}

bool Input::GetControllerButtonUp(int player_num, const std::string& buttoncode) {
	auto player_it = players.find(player_num);

	if (player_it == players.end())
		return false;

	auto it = __buttonname_to_gamecontrollerbutton.find(buttoncode);
	if (it == __buttonname_to_gamecontrollerbutton.end())
		return false;

	Uint8 button = it->second;

	auto controller = controllers.find(player_it->second);
	if (controller == controllers.end()) return false;
	return controller->second.button_states[button] == INPUT_STATE_JUST_UP;
}

float Input::GetJoystickX(int player_num, const std::string& side) {
	auto player_it = players.find(player_num);

	if (player_it == players.end())
		return 0.0f;

	auto controller = controllers.find(player_it->second);
	if (controller == controllers.end()) return false;
	if (side == "left")
		return controller->second.axes[SDL_CONTROLLER_AXIS_LEFTX];
	else if (side == "right")
		return controller->second.axes[SDL_CONTROLLER_AXIS_RIGHTX];
	else
		return 0.0f;
}

float Input::GetJoystickY(int player_num, const std::string& side) {
	auto player_it = players.find(player_num);

	if (player_it == players.end())
		return 0.0f;

	auto controller = controllers.find(player_it->second);
	if (controller == controllers.end()) return false;
	if (side == "left")
		return controller->second.axes[SDL_CONTROLLER_AXIS_LEFTY];
	else if (side == "right")
		return controller->second.axes[SDL_CONTROLLER_AXIS_RIGHTY];
	else
		return 0.0f;
}

void Input::OnControllerEvent(const std::string& event_type, int player_num, int controller_num) {
	// Create a Lua table
	luabridge::LuaRef event_object = luabridge::newTable(ComponentManager::GetLuaState());
	event_object["player_num"] = player_num;
	event_object["controller_num"] = controller_num;

	// Publish the event
	EventBus::Publish(event_type, event_object);
}

