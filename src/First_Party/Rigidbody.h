#pragma once
#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "utility.h"
#include "actor.h"
#include "ComponentManager.h"


class ContactListener : public b2ContactListener {
public:
	void BeginContact(b2Contact* contact) override;
	void EndContact(b2Contact* contact) override;
};

class Collision {
public:
	Actor* other; // The other actor involved in the collision.
	b2Vec2 point;
	b2Vec2 relative_velocity;
	b2Vec2 normal;
};

class Rigidbody
{
public:
	static inline bool world_initialized = false;
	static inline b2World* world = nullptr;
	static inline ContactListener* contact_listener = nullptr;

	std::string type = "Rigidbody";
	std::string key = "???";
	Actor* actor = nullptr;
	bool enabled = true;

	b2Body* body;

	float x = 0.0f;
	float y = 0.0f;
	std::string body_type = "dynamic";
	bool precise = true;
	float gravity_scale = 1.0f;
	float density = 1.0f;
	float angular_friction = 0.3f;

	// rotation stored in degrees
	float rotation = 0.0f;
	bool has_collider = true;
	bool has_trigger = true; 

	// collider properties:
	std::string collider_type = "box";
	float width = 1.0f;
	float height = 1.0f;
	float radius = 0.5f;
	float friction = 0.3f;
	float bounciness = 0.3f;
	int mask_layer = -1;

	// trigger properties:
	std::string trigger_type = "box";
	float trigger_width = 1.0f;
	float trigger_height = 1.0f;
	float trigger_radius = 0.5f;


	Rigidbody();

	Rigidbody(Rigidbody* other);

	void InitializeWorld();

	float DegreesToRadians(const float degrees);

	float RadiansToDegrees(const float radians);

	void OnStart();

	b2Vec2 GetPosition();

	// Return the rotation of the body in degrees
	float GetRotation();

	void CreateFixture();

	void AddForce(const b2Vec2& _force);

	void SetVelocity(const b2Vec2& _velocity);

	b2Vec2 GetVelocity();

	void SetPosition(const b2Vec2& _position);

	void SetRotation(const float degrees_clockwise);

	void SetAngularVelocity(const float degrees_clockwise);

	float GetAngularVelocity();

	void SetGravityScale(const float _gravity_scale);

	float GetGravityScale();

	void SetUpDirection(const b2Vec2& _direction);

	b2Vec2 GetUpDirection();

	void SetRightDirection(const b2Vec2& _direction);

	b2Vec2 GetRightDirection();

	// TODO Copy the rigidbody creation code from slides 51 on in Lab 9 slides

	static void Step();

	void OnDestroy();
};

#endif
