#include "Raycast.h"
#include "ComponentManager.h"
#include "Rigidbody.h"

float RaycastCallback::ReportFixture(b2Fixture* fixture, 
	const b2Vec2& point, const b2Vec2& normal, float fraction) {
	Physics::hitResult.actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);

	// if the actor hit was a phantom fixture, ignore it
	if (Physics::hitResult.actor == nullptr || fixture->GetFilterData().categoryBits == 0x0003)
		return -1.0f;

	Physics::hitSomething = true;
	Physics::hitResult.point = point;
	Physics::hitResult.normal = normal;
	Physics::hitResult.is_trigger = fixture->IsSensor();

	return fraction;
}

float RaycastAllCallback::ReportFixture(b2Fixture* fixture,
	const b2Vec2& point, const b2Vec2& normal, float fraction) {
	Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);

	// if the actor hit was a phantom fixture, ignore it
	if (actor == nullptr || fixture->GetFilterData().categoryBits == 0x0003)
		return -1.0f;

	HitResult hit;
	hit.actor = actor;
	hit.point = point;
	hit.normal = normal;
	hit.is_trigger = fixture->IsSensor();
	Physics::hitResults.push_back(hit);

	return 1.0f; // Return this to continue looking for things along the raycast
}

luabridge::LuaRef Physics::PhysicsRaycast(const b2Vec2& pos, const b2Vec2& dir, const float dist) {
	if (dist <= 0 || !Rigidbody::world_initialized) 
		return luabridge::LuaRef(ComponentManager::GetLuaState());  // Return nil if invalid
	
	b2Vec2 end = pos + dist * dir;
	RaycastCallback callback;
	hitSomething = false;
	Rigidbody::world->RayCast(&callback, pos, end);

	if (!hitSomething) {
		return luabridge::LuaRef(ComponentManager::GetLuaState());  // Return nil if didn't hit anything
	}

	return luabridge::LuaRef(ComponentManager::GetLuaState(), hitResult);
}

luabridge::LuaRef Physics::PhysicsRaycastAll(const b2Vec2& pos, const b2Vec2& dir, const float dist) {
	if (dist <= 0 || !Rigidbody::world_initialized)
		return luabridge::LuaRef(luabridge::newTable(ComponentManager::GetLuaState()));  // Return nil if invalid

	b2Vec2 end = pos + dist * dir;
	RaycastAllCallback callback;
	hitResults.clear();
	Rigidbody::world->RayCast(&callback, pos, end);

	if (hitResults.empty())
		return luabridge::LuaRef(luabridge::newTable(ComponentManager::GetLuaState()));  // Return nil if didn't hit anything

	// Sort results by distance along the ray
	std::sort(hitResults.begin(), hitResults.end(), [&pos](const HitResult& a, const HitResult& b) {
		return (a.point - pos).LengthSquared() < (b.point - pos).LengthSquared();
		});

	luabridge::LuaRef results = luabridge::newTable(ComponentManager::GetLuaState());
	int index = 1; // Lua tables are 1-based

	for (HitResult hit : hitResults) {
		results[index] = hit; // Insert using numeric index
		index++;
	}

	return results;
}
