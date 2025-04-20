#pragma once
#ifndef RAYCAST_H
#define RAYCAST_H

#include "utility.h"

class Actor;

class RaycastCallback : public b2RayCastCallback {
public:
	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;
};

class RaycastAllCallback : public b2RayCastCallback {
public:
	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;
};

class HitResult {
public:
	Actor* actor;
	b2Vec2 point;
	b2Vec2 normal;
	bool is_trigger;
};

class Physics
{
public:
	static inline bool hitSomething = false;
	static inline HitResult hitResult;
	static inline std::vector<HitResult> hitResults;

	static luabridge::LuaRef PhysicsRaycast(const b2Vec2& pos, const b2Vec2& dir, const float dist);

	static luabridge::LuaRef PhysicsRaycastAll(const b2Vec2& pos, const b2Vec2& dir, const float dist);
};

#endif
