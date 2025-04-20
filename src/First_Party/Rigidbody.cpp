#include "Rigidbody.h"

void ContactListener::BeginContact(b2Contact* contact) {
	b2Fixture* fixtureA = contact->GetFixtureA();
	Actor* actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
	b2Fixture* fixtureB = contact->GetFixtureB();
	Actor* actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);

	Collision* collision = new Collision();
	collision->other = actorB;

	b2WorldManifold world_manifold;
	contact->GetWorldManifold(&world_manifold);
	collision->point = world_manifold.points[0];
	collision->relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - 
		fixtureB->GetBody()->GetLinearVelocity();
	collision->normal = world_manifold.normal;

	if (fixtureA->IsSensor() && fixtureB->IsSensor()) {
		collision->point = b2Vec2(-999.0f, -999.0f);
		collision->normal = b2Vec2(-999.0f, -999.0f);
		actorA->OnTriggerEnter(collision);
		collision->other = actorA;
		actorB->OnTriggerEnter(collision);
	}
	else if (!fixtureA->IsSensor() && !fixtureB->IsSensor()) {
		actorA->OnCollisionEnter(collision);
		collision->other = actorA;
		actorB->OnCollisionEnter(collision);
	}
}

void ContactListener::EndContact(b2Contact* contact) {
	b2Fixture* fixtureA = contact->GetFixtureA();
	Actor* actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
	b2Fixture* fixtureB = contact->GetFixtureB();
	Actor* actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);

	Collision* collision = new Collision();
	collision->other = actorB;

	collision->relative_velocity = fixtureA->GetBody()->GetLinearVelocity() -
		fixtureB->GetBody()->GetLinearVelocity();
	collision->point = b2Vec2(-999.0f, -999.0f);
	collision->normal = b2Vec2(-999.0f, -999.0f);

	if (fixtureA->IsSensor() && fixtureB->IsSensor()) {
		actorA->OnTriggerExit(collision);
		collision->other = actorA;
		actorB->OnTriggerExit(collision);
	}
	else if (!fixtureA->IsSensor() && !fixtureB->IsSensor()) {
		actorA->OnCollisionExit(collision);
		collision->other = actorA;
		actorB->OnCollisionExit(collision);
	}
}

Rigidbody::Rigidbody() {
	InitializeWorld();

	body = nullptr;
}

Rigidbody::Rigidbody(Rigidbody* other) {
	InitializeWorld();

	// TODO: Overhaul this entirely
	*this = *other;
	body = nullptr;
	/*
	x = (*other->componentRef)["x"];
	y = (*other->componentRef)["y"];
	body_type = (*other->componentRef)["body_type"].tostring();
	precise = (*other->componentRef)["precise"];
	gravity_scale = (*other->componentRef)["gravity_scale"];
	density = (*other->componentRef)["density"];
	angular_friction = (*other->componentRef)["angular_friction"];
	rotation = (*other->componentRef)["rotation"];
	has_collider = (*other->componentRef)["has_collider"].cast<bool>();
	has_trigger = (*other->componentRef)["has_trigger"].cast<bool>();
	*/
}

void Rigidbody::InitializeWorld() {
	if (!world_initialized) {
		world = new b2World(b2Vec2(0.0f, 9.8f));
		contact_listener = new ContactListener();
		world->SetContactListener(contact_listener);

		world_initialized = true;
	}
}

float Rigidbody::DegreesToRadians(const float degrees) {
	return degrees * (b2_pi / 180.0f);
}

float Rigidbody::RadiansToDegrees(const float radians) {
	return radians * (180.0f / b2_pi);
}

void Rigidbody::OnStart() {
	b2BodyDef body_def;
	if (body_type == "dynamic")
		body_def.type = b2_dynamicBody;
	else if (body_type == "kinematic")
		body_def.type = b2_kinematicBody;
	else if (body_type == "static")
		body_def.type = b2_staticBody;

	body_def.position = b2Vec2(x, y);
	body_def.bullet = precise;
	body_def.gravityScale = gravity_scale;
	body_def.angularDamping = angular_friction;
	body_def.angle = DegreesToRadians(rotation);

	body = world->CreateBody(&body_def);

	CreateFixture();
}

b2Vec2 Rigidbody::GetPosition() {
	if (body == nullptr)
		return b2Vec2(x, y);

	return body->GetPosition();
}

float Rigidbody::GetRotation() {
	float rotation_radians = body->GetAngle();
	return RadiansToDegrees(rotation_radians);
}

void Rigidbody::CreateFixture() {
	if (!has_collider && !has_trigger) {
		b2PolygonShape phantom_shape;
		phantom_shape.SetAsBox(width * 0.5f, height * 0.5f);

		b2FixtureDef phantom_fixture_def;
		phantom_fixture_def.shape = &phantom_shape;
		phantom_fixture_def.density = density;
		phantom_fixture_def.userData.pointer = reinterpret_cast<uintptr_t> (actor);
		phantom_fixture_def.isSensor = true;
		// **Set the collision filter for triggers**
		phantom_fixture_def.filter.categoryBits = 0x0003; // Phantom category
		phantom_fixture_def.filter.maskBits = 0x0000; // Colliders & Triggers
		body->CreateFixture(&phantom_fixture_def);
		return;
	}
	if (has_collider) {
		b2Shape* shape = nullptr;
		if (collider_type == "box") {
			b2PolygonShape* my_shape = new b2PolygonShape;
			my_shape->SetAsBox(width * 0.5f, height * 0.5f);
			shape = my_shape;
		}
		else if (collider_type == "circle") {
			b2CircleShape* circle_shape = new b2CircleShape();
			circle_shape->m_radius = radius;
			shape = circle_shape;
		}

		b2FixtureDef fixture;
		fixture.shape = shape;
		fixture.density = density;
		fixture.isSensor = false;
		fixture.restitution = bounciness;
		fixture.friction = friction;
		fixture.userData.pointer = reinterpret_cast<uintptr_t> (actor);

		// --- Layer Mask Setup ---
		uint16 category = (mask_layer >= 0) ? (1 << mask_layer) : 0x0001;
		uint16 mask = 0xFFFF;
		if (mask_layer >= 0) {
			mask &= ~(1 << mask_layer); // don't collide with same layer
		}

		// **Set the collision filter for colliders** 
		fixture.filter.categoryBits = category; // Collider category
		fixture.filter.maskBits = mask;     // Colliders only collide with other colliders

		body->CreateFixture(&fixture);
	}
	if (has_trigger) {
		b2Shape* shape = nullptr;
		if (trigger_type == "box") {
			b2PolygonShape* my_shape = new b2PolygonShape;
			my_shape->SetAsBox(trigger_width * 0.5f, trigger_height * 0.5f);
			shape = my_shape;
		}
		else if (trigger_type == "circle") {
			b2CircleShape* circle_shape = new b2CircleShape();
			circle_shape->m_radius = trigger_radius;
			shape = circle_shape;
		}

		b2FixtureDef fixture;
		fixture.shape = shape;
		fixture.density = density;
		fixture.isSensor = true;
		fixture.restitution = bounciness;
		fixture.friction = friction;
		fixture.userData.pointer = reinterpret_cast<uintptr_t> (actor);

		// **Set the collision filter for triggers**
		fixture.filter.categoryBits = 0x0002; // Trigger category
		fixture.filter.maskBits = 0x0002;     // Triggers only interact with other triggers

		body->CreateFixture(&fixture);
	}
}

void Rigidbody::AddForce(const b2Vec2& _force) {
	body->ApplyForceToCenter(_force, true);
}

void Rigidbody::SetVelocity(const b2Vec2& _velocity) {
	body->SetLinearVelocity(_velocity);
}

b2Vec2 Rigidbody::GetVelocity() {
	return body->GetLinearVelocity();
}

void Rigidbody::SetPosition(const b2Vec2& _position) {
	if (body != nullptr)
		body->SetTransform(_position, body->GetAngle());
	else {
		x = _position.x;
		y = _position.y;
	}
}

void Rigidbody::SetRotation(const float degrees_clockwise) {
	rotation = degrees_clockwise;
	body->SetTransform(GetPosition(), DegreesToRadians(rotation));
}

void Rigidbody::SetAngularVelocity(const float degrees_clockwise) {
	body->SetAngularVelocity(DegreesToRadians(degrees_clockwise));
}

float Rigidbody::GetAngularVelocity() {
	return RadiansToDegrees(body->GetAngularVelocity());
}

void Rigidbody::SetGravityScale(const float _gravity_scale) {
	gravity_scale = _gravity_scale;
	body->SetGravityScale(gravity_scale);
}

float Rigidbody::GetGravityScale() {
	return body->GetGravityScale();
}

void Rigidbody::SetUpDirection(const b2Vec2& _direction) {
	b2Vec2 normalized_direction = _direction;
	normalized_direction.Normalize();
	body->SetTransform(GetPosition(), (glm::atan(normalized_direction.x, -normalized_direction.y)));
}

b2Vec2 Rigidbody::GetUpDirection() {
	float angle = body->GetAngle() - (b2_pi / 2.0f);
	//std::cout << "Angle in radians is " << angle << "\n";
	b2Vec2 result = b2Vec2(glm::cos(angle), glm::sin(angle));
	result.Normalize();
	//std::cout << "Result's x component is " << result.x << " and the y component is " << result.y << "\n";
	return result;
}

void Rigidbody::SetRightDirection(const b2Vec2& _direction) {
	b2Vec2 normalized_direction = _direction;
	normalized_direction.Normalize();
	body->SetTransform(GetPosition(), (glm::atan(normalized_direction.x, -normalized_direction.y) - (b2_pi / 2.0f))); 
}

b2Vec2 Rigidbody::GetRightDirection() {
	float angle = body->GetAngle();
	b2Vec2 result = b2Vec2(glm::cos(angle), glm::sin(angle));
	result.Normalize();
	return result;
}

void Rigidbody::Step() {
	if (!world_initialized)
		return;

	world->Step(1.0f / 60.0f, 8, 3);
}

void Rigidbody::OnDestroy() {
	world->DestroyBody(body);
}
