#pragma once
#ifndef INPUT_H
#define INPUT_H

#include <filesystem>
#include <unordered_map>
#include "utility.h"

namespace fs = std::filesystem;

enum INPUT_STATE {
    INPUT_STATE_UP,
    INPUT_STATE_JUST_DOWN,
    INPUT_STATE_DOWN,
    INPUT_STATE_JUST_UP
};

struct Controller {
	int id;  // SDL device ID
	SDL_GameController* handle = nullptr;
	SDL_JoystickID instanceID = -1;  // Unique ID for hotplugging/removal
	int player_num = -1;

	// State info
	std::vector<float> axes;        // Analog sticks
	//std::vector<bool> buttons;      // Buttons
	std::unordered_map<Uint8, INPUT_STATE> button_states;
	std::vector<Uint8> just_became_down_buttons;
	std::vector<Uint8> just_became_up_buttons;
	std::vector<int> hats;          // D-pad hat (if used)
};

class Input
{
public:

    // Public method to access the single instance of the class
    static Input& getInstance() {
        static Input instance; // Guaranteed to be created only once
        return instance;
    }

    // Call before main loop begins
    static void Init();

	static void InitControllers();

	static void AddController(int controller_num);

    // Call every frame at start of event loop
    static bool ProcessEvent(const SDL_Event& e);

    // Call at very end of frame
    static void LateUpdate();

    static bool GetKey(const std::string& keycode);
    static bool GetKeyDown(const std::string& keycode);
    static bool GetKeyUp(const std::string& keycode);

    static glm::vec2 GetMousePosition();

    static bool GetMouseButton(Uint8 button_num);
    static bool GetMouseButtonDown(Uint8 button_num);
    static bool GetMouseButtonUp(Uint8 button_num);
    static float GetMouseScrollDelta();
	static void HideCursor();
	static void ShowCursor();

	// The user passes in the player_num:
	// - If the player is already associated with a connected controller,
	//   the function just returns true
	// - If there is an unassigned controller,the player num is now associated 
	//   with that controller, and the function returns true.
	// - If there is no unassigned controller, the function returns false.
	static bool ConnectPlayerToController(int player_num);

	// Frees up the controller connected to player_num if one is connected.
	static void DisconnectPlayerFromController(int player_num);

	// Returns -1 if no controller is associated with that player
	static int GetControllerIDByPlayerNum(int player_num);

	static bool ControllerConnected(int player_num);

	static int GetNumberOfConnectedControllers();

	static bool GetControllerButton(int player_num, const std::string& buttoncode);
	static bool GetControllerButtonDown(int player_num, const std::string& buttoncode);
	static bool GetControllerButtonUp(int player_num, const std::string& buttoncode);

	// Returns 0.0f if the player_num is not associated with a controller
	// side == "left" for left stick and "right" for right stick
	static float GetJoystickX(int player_num, const std::string& side);

	// Returns 0.0f if the player_num is not associated with a controller
	// side == "left" for left stick and "right" for right stick
	static float GetJoystickY(int player_num, const std::string& side);

	static void OnControllerEvent(const std::string& event_type, int player_num, int controller_num);


private:
    static inline std::unordered_map<SDL_Scancode, INPUT_STATE> keyboard_states;
    static inline std::vector<SDL_Scancode> just_became_down_scancodes;
    static inline std::vector<SDL_Scancode> just_became_up_scancodes;

    static inline glm::vec2 mouse_position;
    static inline std::unordered_map<Uint8, INPUT_STATE> mouse_states;
    static inline std::vector<Uint8> just_became_down_mouse_buttons;
    static inline std::vector<Uint8> just_became_up_mouse_buttons;

    static inline float mouse_scroll_this_frame = 0;

	// string to keycode mapper
	const static inline std::unordered_map<std::string, SDL_Scancode> __keycode_to_scancode = {
		// Directional (arrow) Keys
		{"up", SDL_SCANCODE_UP},
		{"down", SDL_SCANCODE_DOWN},
		{"right", SDL_SCANCODE_RIGHT},
		{"left", SDL_SCANCODE_LEFT},

		// Misc Keys
		{"escape", SDL_SCANCODE_ESCAPE},

		// Modifier Keys
		{"lshift", SDL_SCANCODE_LSHIFT},
		{"rshift", SDL_SCANCODE_RSHIFT},
		{"lctrl", SDL_SCANCODE_LCTRL},
		{"rctrl", SDL_SCANCODE_RCTRL},
		{"lalt", SDL_SCANCODE_LALT},
		{"ralt", SDL_SCANCODE_RALT},

		// Editing Keys
		{"tab", SDL_SCANCODE_TAB},
		{"return", SDL_SCANCODE_RETURN},
		{"enter", SDL_SCANCODE_RETURN},
		{"backspace", SDL_SCANCODE_BACKSPACE},
		{"delete", SDL_SCANCODE_DELETE},
		{"insert", SDL_SCANCODE_INSERT},

		// Character Keys
		{"space", SDL_SCANCODE_SPACE},
		{"a", SDL_SCANCODE_A},
		{"b", SDL_SCANCODE_B},
		{"c", SDL_SCANCODE_C},
		{"d", SDL_SCANCODE_D},
		{"e", SDL_SCANCODE_E},
		{"f", SDL_SCANCODE_F},
		{"g", SDL_SCANCODE_G},
		{"h", SDL_SCANCODE_H},
		{"i", SDL_SCANCODE_I},
		{"j", SDL_SCANCODE_J},
		{"k", SDL_SCANCODE_K},
		{"l", SDL_SCANCODE_L},
		{"m", SDL_SCANCODE_M},
		{"n", SDL_SCANCODE_N},
		{"o", SDL_SCANCODE_O},
		{"p", SDL_SCANCODE_P},
		{"q", SDL_SCANCODE_Q},
		{"r", SDL_SCANCODE_R},
		{"s", SDL_SCANCODE_S},
		{"t", SDL_SCANCODE_T},
		{"u", SDL_SCANCODE_U},
		{"v", SDL_SCANCODE_V},
		{"w", SDL_SCANCODE_W},
		{"x", SDL_SCANCODE_X},
		{"y", SDL_SCANCODE_Y},
		{"z", SDL_SCANCODE_Z},
		{"0", SDL_SCANCODE_0},
		{"1", SDL_SCANCODE_1},
		{"2", SDL_SCANCODE_2},
		{"3", SDL_SCANCODE_3},
		{"4", SDL_SCANCODE_4},
		{"5", SDL_SCANCODE_5},
		{"6", SDL_SCANCODE_6},
		{"7", SDL_SCANCODE_7},
		{"8", SDL_SCANCODE_8},
		{"9", SDL_SCANCODE_9},
		{"/", SDL_SCANCODE_SLASH},
		{";", SDL_SCANCODE_SEMICOLON},
		{"=", SDL_SCANCODE_EQUALS},
		{"-", SDL_SCANCODE_MINUS},
		{".", SDL_SCANCODE_PERIOD},
		{",", SDL_SCANCODE_COMMA},
		{"[", SDL_SCANCODE_LEFTBRACKET},
		{"]", SDL_SCANCODE_RIGHTBRACKET},
		{"\\", SDL_SCANCODE_BACKSLASH},
		{"'", SDL_SCANCODE_APOSTROPHE}
	};

	static inline std::unordered_map<SDL_JoystickID, Controller> controllers;

	// This is the queue of controllers that are connected, but haven't been attached to a player
	static inline std::queue<SDL_JoystickID> unconnected_controller_ids;

	// This is the queue of controller ids that have been freed because the controllers disconnected
	//static inline std::queue<SDL_JoystickID> disconnected_controller_ids;
	static inline std::unordered_map<int, SDL_JoystickID> players;

	static inline std::vector<SDL_JoystickID> just_became_down_controllers;
	static inline std::vector<SDL_JoystickID> just_became_up_controllers;

	// string to game controller button mapper
	const static inline std::unordered_map<std::string, SDL_GameControllerButton> __buttonname_to_gamecontrollerbutton = {
		// Face Buttons
		{"a", SDL_CONTROLLER_BUTTON_A},
		{"b", SDL_CONTROLLER_BUTTON_B},
		{"x", SDL_CONTROLLER_BUTTON_X},
		{"y", SDL_CONTROLLER_BUTTON_Y},

		// Shoulder Buttons
		{"leftshoulder", SDL_CONTROLLER_BUTTON_LEFTSHOULDER},
		{"rightshoulder", SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},

		// Stick Clicks
		{"leftstick", SDL_CONTROLLER_BUTTON_LEFTSTICK},
		{"rightstick", SDL_CONTROLLER_BUTTON_RIGHTSTICK},

		// D-Pad
		{"dpup", SDL_CONTROLLER_BUTTON_DPAD_UP},
		{"dpdown", SDL_CONTROLLER_BUTTON_DPAD_DOWN},
		{"dpleft", SDL_CONTROLLER_BUTTON_DPAD_LEFT},
		{"dpright", SDL_CONTROLLER_BUTTON_DPAD_RIGHT},

		// Menu/System Buttons
		{"back", SDL_CONTROLLER_BUTTON_BACK},
		{"start", SDL_CONTROLLER_BUTTON_START},
		{"guide", SDL_CONTROLLER_BUTTON_GUIDE}, // e.g., Xbox logo / PS home

		// Touchpad (rarely used)
		{"touchpad", SDL_CONTROLLER_BUTTON_TOUCHPAD},

		// Misc
		{"misc1", SDL_CONTROLLER_BUTTON_MISC1}, // Unused on most controllers

		// Max (for bounds checking or iteration)
		{"max", SDL_CONTROLLER_BUTTON_MAX}
	};

    // Private constructor and destructor to prevent multiple instances
    Input() {}
    ~Input() = default;

    // Delete copy constructor and assignment operator to prevent copying
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;
};

#endif