#include "ComponentManager.h"
#include "actor.h"
#include "game.h"
#include "renderer.h"
#include "Rigidbody.h"
#include "Raycast.h"
#include "EventBus.h"
#include "ParticleSystem.h"

Component::Component(lua_State* lua_state, const std::string& filePath, const std::string& componentName) :
	hasStart(false), hasUpdate(false), hasLateUpdate(false) {
	
	// Load Lua file
	if (luaL_dofile(lua_state, filePath.c_str()) != LUA_OK) {
		std::cout << "problem with lua file " << componentName;
		exit(0);
	}
	
	luabridge::LuaRef parentTable = luabridge::getGlobal(lua_state, componentName.c_str());

	componentRef = std::make_shared<luabridge::LuaRef>(luabridge::newTable(lua_state));

	ComponentManager::EstablishInheritance(*componentRef, parentTable);

	// Get the Lua component
	//luabridge::LuaRef luaComponent = luabridge::getGlobal(lua_state, componentName.c_str());

	// Store as shared pointer
	//componentRef = std::make_shared<luabridge::LuaRef>(luaComponent);
	(*componentRef)["enabled"] = true;
	type = componentName;

	isCPPComponent = false;

	// Check for function existence
	hasStart = (*componentRef)["OnStart"].isFunction();
	hasUpdate = (*componentRef)["OnUpdate"].isFunction();
	hasLateUpdate = (*componentRef)["OnLateUpdate"].isFunction();
	hasOnDestroy = (*componentRef)["OnDestroy"].isFunction();
	hasOnCollisionEnter = (*componentRef)["OnCollisionEnter"].isFunction();
	hasOnCollisionExit = (*componentRef)["OnCollisionExit"].isFunction();
	hasOnTriggerEnter = (*componentRef)["OnTriggerEnter"].isFunction();
	hasOnTriggerExit = (*componentRef)["OnTriggerExit"].isFunction();
}

bool Component::IsEnabled() {
	//luabridge::LuaRef enabled = (*componentRef)["enabled"];

	//return (enabled.cast<bool>() == true);

	return (*componentRef)["enabled"].cast<bool>();// == true;
}

Component::Component(const Component& other, lua_State* lua_state) {
	componentRef = std::make_shared<luabridge::LuaRef>(luabridge::newTable(lua_state));

	ComponentManager::EstablishInheritance(*componentRef, *other.componentRef);

	(*componentRef)["enabled"] = true;
	type = other.type;

	isCPPComponent = false;

	// Check for function existence
	hasStart = other.hasStart;
	hasUpdate = other.hasUpdate;
	hasLateUpdate = other.hasLateUpdate;
	hasOnDestroy = other.hasOnDestroy;
	hasOnCollisionEnter = other.hasOnCollisionEnter;
	hasOnCollisionExit = other.hasOnCollisionExit;
	hasOnTriggerEnter = other.hasOnTriggerEnter;
	hasOnTriggerExit = other.hasOnTriggerExit;
}

Component::Component() {
	type = "???";
	isCPPComponent = false;
	hasStart = false;
	hasUpdate = false;
	hasLateUpdate = false;
	hasOnDestroy = false;
	hasOnCollisionEnter = false;
	hasOnCollisionExit = false;
	hasOnTriggerEnter = false;
	hasOnTriggerExit = false;
}

void ComponentManager::Init() {
	lua_state = luaL_newstate();
	luaL_openlibs(lua_state);

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Debug")
		.addFunction("Log", ComponentManager::CppDebugLog)
		.endNamespace();

	luabridge::getGlobalNamespace(lua_state)
		.beginClass<glm::vec2>("vec2")
		.addProperty("x", &glm::vec2::x)
		.addProperty("y", &glm::vec2::y)
		.endClass();

	luabridge::getGlobalNamespace(lua_state)
		.beginClass<Actor>("Actor")
		.addFunction("GetName", &Actor::GetName)
		.addFunction("GetID", &Actor::GetID)
		.addFunction("GetComponentByKey", &Actor::GetComponentByKey)
		.addFunction("GetComponent", &Actor::GetComponent)
		.addFunction("GetComponents", &Actor::GetComponents)
		.addFunction("AddComponent", &Actor::AddComponent)
		.addFunction("RemoveComponent", &Actor::RemoveComponent)
		.endClass();


	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Actor")
		.addFunction("Find", &Game::Find)
		.addFunction("FindAll", &Game::FindAll)
		.addFunction("Instantiate", &Game::Instantiate)
		.addFunction("Destroy", &Game::Destroy)
		.endNamespace();

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Application")
		.addFunction("GetFrame", &Helper::GetFrameNumber) 
		.addFunction("GetTime", &Game::GetTime)
		.addFunction("Quit", &Game::Quit)
		.addFunction("Sleep", &Game::Sleep)
		.addFunction("OpenURL", &Game::OpenURL)
		.endNamespace();

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Input")
		.addFunction("GetKey", &Input::GetKey)
		.addFunction("GetKeyDown", &Input::GetKeyDown)
		.addFunction("GetKeyUp", &Input::GetKeyUp)
		.addFunction("GetMousePosition", &Input::GetMousePosition)
		.addFunction("GetMouseButton", &Input::GetMouseButton)
		.addFunction("GetMouseButtonDown", &Input::GetMouseButtonDown)
		.addFunction("GetMouseButtonUp", &Input::GetMouseButtonUp)
		.addFunction("GetMouseScrollDelta", &Input::GetMouseScrollDelta)
		.addFunction("HideCursor", &Input::HideCursor)
		.addFunction("ShowCursor", &Input::ShowCursor)
		.addFunction("ConnectPlayerToController", &Input::ConnectPlayerToController)
		.addFunction("DisconnectPlayerFromController", &Input::DisconnectPlayerFromController)
		.addFunction("GetControllerIDByPlayerNum", &Input::GetControllerIDByPlayerNum)
		.addFunction("ControllerConnected", &Input::ControllerConnected)
		.addFunction("GetNumberOfConnectedControllers", &Input::GetNumberOfConnectedControllers)
		.addFunction("GetControllerButton", &Input::GetControllerButton)
		.addFunction("GetControllerButtonDown", &Input::GetControllerButtonDown)
		.addFunction("GetControllerButtonUp", &Input::GetControllerButtonUp)
		.addFunction("GetJoystickX", &Input::GetJoystickX)
		.addFunction("GetJoystickY", &Input::GetJoystickY)
		.endNamespace();

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Text")
		.addFunction("Draw", &Renderer::DrawText)
		.endNamespace();

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Audio")
		.addFunction("Play", &AudioDB::Play)
		.addFunction("Halt", &AudioDB::HaltChannel)
		.addFunction("SetVolume", &AudioDB::SetVolume)
		.endNamespace();

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Image")
		.addFunction("DrawUI", &Renderer::DrawUI)
		.addFunction("DrawUIEx", &Renderer::DrawUIEx)
		.addFunction("Draw", &Renderer::DrawImage)
		.addFunction("DrawEx", &Renderer::DrawImageEx)
		.addFunction("DrawPixel", &Renderer::DrawPixel)
		.endNamespace();

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Camera")
		.addFunction("SetPosition", &Renderer::CameraSetPosition)
		.addFunction("GetPositionX", &Renderer::CameraGetPositionX)
		.addFunction("GetPositionY", &Renderer::CameraGetPositionY)
		.addFunction("SetZoom", &Renderer::CameraSetZoom)
		.addFunction("GetZoom", &Renderer::CameraGetZoom)
		.addFunction("GetCameraWidth", &GameData::GetCameraWidth)
		.addFunction("GetCameraHeight", &GameData::GetCameraHeight)
		.endNamespace();

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Scene")
		.addFunction("Load", &Game::QueueSceneLoad)
		.addFunction("GetCurrent", &Game::GetCurrentSceneName)
		.addFunction("DontDestroy", &Game::DontDestroyOnLoad)
		.endNamespace();

	luabridge::getGlobalNamespace(lua_state)
		.beginClass<b2Vec2>("Vector2")
		.addConstructor<void(*) (float, float)>()
		.addProperty("x", &b2Vec2::x)
		.addProperty("y", &b2Vec2::y)
		.addFunction("Normalize", &b2Vec2::Normalize)
		.addFunction("Length", &b2Vec2::Length)
		.addFunction("__add", &b2Vec2::operator_add) 
		.addFunction("__sub", &b2Vec2::operator_sub)
		.addFunction("__mul", &b2Vec2::operator_mul)
		.addStaticFunction("Distance", static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Distance)) 
		.addStaticFunction("Dot", static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Dot))
		.endClass();

	luabridge::getGlobalNamespace(lua_state)
		.beginClass<Rigidbody>("Rigidbody")
		.addConstructor<void (*) (void)>()
		.addData("enabled", &Rigidbody::enabled)
		.addData("key", &Rigidbody::key)
		.addData("type", &Rigidbody::type)
		.addData("actor", &Rigidbody::actor)
		.addData("x", &Rigidbody::x)
		.addData("y", &Rigidbody::y)
		.addData("body_type", &Rigidbody::body_type)
		.addData("precise", &Rigidbody::precise)
		.addData("gravity_scale", &Rigidbody::gravity_scale)
		.addData("density", &Rigidbody::density)
		.addData("angular_friction", &Rigidbody::angular_friction)
		.addData("rotation", &Rigidbody::rotation)
		.addData("has_collider", &Rigidbody::has_collider)
		.addData("has_trigger", &Rigidbody::has_trigger)
		.addData("collider_type", &Rigidbody::collider_type)
		.addData("width", &Rigidbody::width)
		.addData("height", &Rigidbody::height)
		.addData("radius", &Rigidbody::radius)
		.addData("friction", &Rigidbody::friction)
		.addData("bounciness", &Rigidbody::bounciness)
		.addData("mask_layer", &Rigidbody::mask_layer)
		.addData("trigger_type", &Rigidbody::trigger_type)
		.addData("trigger_width", &Rigidbody::trigger_width)
		.addData("trigger_height", &Rigidbody::trigger_height)
		.addData("trigger_radius", &Rigidbody::trigger_radius)

		.addFunction("OnStart", &Rigidbody::OnStart)
		.addFunction("OnDestroy", &Rigidbody::OnDestroy)
		.addFunction("GetPosition", &Rigidbody::GetPosition)
		.addFunction("GetRotation", &Rigidbody::GetRotation)
		.addFunction("AddForce", &Rigidbody::AddForce)
		.addFunction("SetVelocity", &Rigidbody::SetVelocity)
		.addFunction("SetPosition", &Rigidbody::SetPosition)
		.addFunction("SetRotation", &Rigidbody::SetRotation)
		.addFunction("SetAngularVelocity", &Rigidbody::SetAngularVelocity)
		.addFunction("SetGravityScale", &Rigidbody::SetGravityScale)
		.addFunction("SetUpDirection", &Rigidbody::SetUpDirection)
		.addFunction("SetRightDirection", &Rigidbody::SetRightDirection)
		.addFunction("GetVelocity", &Rigidbody::GetVelocity)
		.addFunction("GetAngularVelocity", &Rigidbody::GetAngularVelocity)
		.addFunction("GetGravityScale", &Rigidbody::GetGravityScale)
		.addFunction("GetUpDirection", &Rigidbody::GetUpDirection)
		.addFunction("GetRightDirection", &Rigidbody::GetRightDirection)
		.endClass();

	luabridge::getGlobalNamespace(lua_state)
		.beginClass<Collision>("Collision")
		.addProperty("other", &Collision::other)
		.addProperty("point", &Collision::point)
		.addProperty("relative_velocity", &Collision::relative_velocity)
		.addProperty("normal", &Collision::normal)
		.endClass();

	luabridge::getGlobalNamespace(lua_state)
		.beginClass<HitResult>("HitResult")
		.addProperty("actor", &HitResult::actor)
		.addProperty("point", &HitResult::point)
		.addProperty("normal", &HitResult::normal)
		.addProperty("is_trigger", &HitResult::is_trigger)
		.endClass();

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Physics")
		.addFunction("Raycast", &Physics::PhysicsRaycast)
		.addFunction("RaycastAll", &Physics::PhysicsRaycastAll)
		.endNamespace();

	luabridge::getGlobalNamespace(lua_state)
		.beginNamespace("Event")
		.addFunction("Subscribe", &EventBus::Subscribe)
		.addFunction("Publish", [](lua_State* L) {
			int argCount = lua_gettop(L); // Number of arguments passed from Lua

			if (argCount == 1) {
				// Publish(event_type) with only an event name
				std::string event_type = luabridge::Stack<std::string>::get(L, 1);
				EventBus::Publish(event_type);
			}
			else if (argCount == 2) {
				// Publish(event_type, event_object) with an event object
				std::string event_type = luabridge::Stack<std::string>::get(L, 1);
				luabridge::LuaRef event_object = luabridge::Stack<luabridge::LuaRef>::get(L, 2);
				EventBus::Publish(event_type, event_object);
			}
			else {
				luaL_error(L, "Event.Publish() expects 1 or 2 arguments."); // Lua error
			}

			return 0; // Lua functions return the number of return values (0 in this case)
			})
		.addFunction("Unsubscribe", &EventBus::Unsubscribe)
		.endNamespace();

	luabridge::getGlobalNamespace(lua_state)
		.beginClass<ParticleSystem>("ParticleSystem")
		.addConstructor<void (*) (void)>()
		.addData("enabled", &ParticleSystem::enabled)
		.addData("key", &ParticleSystem::key)
		.addData("type", &ParticleSystem::type)
		.addData("actor", &ParticleSystem::actor)
		.addData("emit_angle_min", &ParticleSystem::emit_angle_min)
		.addData("emit_angle_max", &ParticleSystem::emit_angle_max)
		.addData("emit_radius_min", &ParticleSystem::emit_radius_min)
		.addData("emit_radius_max", &ParticleSystem::emit_radius_max)
		.addData("start_scale_min", &ParticleSystem::start_scale_min)
		.addData("start_scale_max", &ParticleSystem::start_scale_max)
		.addData("rotation_min", &ParticleSystem::rotation_min)
		.addData("rotation_max", &ParticleSystem::rotation_max)
		.addData("start_speed_min", &ParticleSystem::start_speed_min)
		.addData("start_speed_max", &ParticleSystem::start_speed_max)
		.addData("rotation_speed_min", &ParticleSystem::rotation_speed_min)
		.addData("rotation_speed_max", &ParticleSystem::rotation_speed_max)
		.addData("x", &ParticleSystem::x)
		.addData("y", &ParticleSystem::y)
		.addData("frames_between_bursts", &ParticleSystem::frames_between_bursts)
		.addData("burst_quantity", &ParticleSystem::burst_quantity)
		.addData("start_color_r", &ParticleSystem::start_color_r)
		.addData("start_color_g", &ParticleSystem::start_color_g)
		.addData("start_color_b", &ParticleSystem::start_color_b)
		.addData("start_color_a", &ParticleSystem::start_color_a)
		.addData("image", &ParticleSystem::image)
		.addData("sorting_order", &ParticleSystem::sorting_order)
		.addData("duration_frames", &ParticleSystem::duration_frames)
		.addData("gravity_scale_x", &ParticleSystem::gravity_scale_x)
		.addData("gravity_scale_y", &ParticleSystem::gravity_scale_y)
		.addData("drag_factor", &ParticleSystem::drag_factor)
		.addData("angular_drag_factor", &ParticleSystem::angular_drag_factor)
		.addData("end_scale", &ParticleSystem::end_scale)
		.addData("end_color_r", &ParticleSystem::end_color_r)
		.addData("end_color_g", &ParticleSystem::end_color_g)
		.addData("end_color_b", &ParticleSystem::end_color_b)
		.addData("end_color_a", &ParticleSystem::end_color_a)

		.addFunction("OnStart", &ParticleSystem::OnStart)
		.addFunction("OnUpdate", &ParticleSystem::OnUpdate)
		.addFunction("Stop", &ParticleSystem::Stop)
		.addFunction("Play", &ParticleSystem::Play)
		.addFunction("Burst", &ParticleSystem::Burst)
		.endClass();
}

void ComponentManager::EstablishInheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table) {
	// We must create a new metatable to establish inheritance in lua.
	luabridge::LuaRef new_metatable = luabridge::newTable(lua_state);
	new_metatable["__index"] = parent_table;

	// We must use the raw lua C-API (lua stack) to perform a "setmetatable" operation.
	instance_table.push(lua_state);
	new_metatable.push(lua_state);
	lua_setmetatable(lua_state, -2);
	lua_pop(lua_state, 1);
}

Component* ComponentManager::GetComponentInstance(std::string componentName) {
	if (componentName == "Rigidbody")
		return CreateRigidbody();
	if (componentName == "ParticleSystem")
		return CreateParticleSystem();

	// If the component already exists, retrieve it and return
	if (luaComponents.find(componentName) != luaComponents.end()) {
		//Component* comp = new Component(*luaComponents[componentName], lua_state);
		return GetTemplatedComponentInstance(luaComponents[componentName]);
	}

	// Get the path of the current executable
	fs::path exePath = fs::current_path();

	fs::path componentDir = exePath / "resources" / "component_types" / (componentName + ".lua");

	if (!fs::exists(componentDir)) {
		std::cout << "error: failed to locate component " << componentName;
		exit(0);
	}

	//components[componentName] = IMG_LoadTexture(renderer, imageDir.string().c_str());
	Component* component = new Component(lua_state, componentDir.string(), componentName);

	luaComponents[componentName] = component;

	Component* comp = new Component(*component, lua_state);

	return comp;
}

Component* ComponentManager::GetTemplatedComponentInstance(Component* otherComponent) {
	if (otherComponent->isCPPComponent)
		return GetTemplatedCPPComponent(otherComponent);

	return new Component(*otherComponent, lua_state);
}

Component* ComponentManager::GetTemplatedCPPComponent(Component* otherComponent) {
	if (otherComponent->type == "Rigidbody")
		return CreateRigidbodyCopy(otherComponent);
	if (otherComponent->type == "ParticleSystem")
		return CreateParticleSystemCopy(otherComponent);
	return new Component();
}

lua_State* ComponentManager::GetLuaState() {
	return lua_state;
}

void ComponentManager::CppDebugLog(const std::string message) {
	std::cout << message << "\n";
}

Component* ComponentManager::CreateRigidbody() {
	Component* component = new Component();
	Rigidbody* rigidbody = new Rigidbody();
	//luabridge::LuaRef componentRef(lua_state, rigidbody);
	component->componentRef = std::make_shared<luabridge::LuaRef>(lua_state, rigidbody);

	//component->rigidbody_ptr = rigidbody;
	(*component->componentRef)["enabled"] = true;
	component->type = "Rigidbody";
	component->isCPPComponent = true;
	component->hasStart = true;
	component->hasOnDestroy = true;
	return component;
}

Component* ComponentManager::CreateRigidbodyCopy(Component* otherComponent) {
	Component* component = new Component();

	Rigidbody* rigidbody = new Rigidbody(otherComponent->componentRef->cast<Rigidbody*>());
	
	component->componentRef = std::make_shared<luabridge::LuaRef>(lua_state, rigidbody);

	//component->rigidbody_ptr = rigidbody;
	(*component->componentRef)["enabled"] = true;
	component->type = "Rigidbody";
	component->isCPPComponent = true;
	component->hasStart = true;
	component->hasOnDestroy = true;
	return component;
}

Component* ComponentManager::CreateParticleSystem() {
	Component* component = new Component();
	ParticleSystem* particleSystem = new ParticleSystem();
	//luabridge::LuaRef componentRef(lua_state, rigidbody);
	component->componentRef = std::make_shared<luabridge::LuaRef>(lua_state, particleSystem);

	(*component->componentRef)["enabled"] = true;
	component->type = "ParticleSystem";
	component->isCPPComponent = true;
	component->hasStart = true;
	component->hasUpdate = true;
	return component;
}

Component* ComponentManager::CreateParticleSystemCopy(Component* otherComponent) {
	Component* component = new Component();

	ParticleSystem* particleSystem = new ParticleSystem(otherComponent->componentRef->cast<ParticleSystem*>());

	component->componentRef = std::make_shared<luabridge::LuaRef>(lua_state, particleSystem);

	(*component->componentRef)["enabled"] = true;
	component->type = "ParticleSystem";
	component->isCPPComponent = true;
	component->hasStart = true;
	component->hasUpdate = true;
	return component;
}
