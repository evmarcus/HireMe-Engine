#pragma once
#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include "utility.h"
#include "actor.h"
#include "ComponentManager.h"

class ParticlePos {
public:
	float x = 0.0f, y = 0.0f;

	ParticlePos() {}

	ParticlePos(float _x, float _y) : x(_x), y(_y) {}
};

class ParticleVelocity {
public:
	float x_vel = 0.0f, y_vel = 0.0f;

	ParticleVelocity() {}

	ParticleVelocity(float _x, float _y) : x_vel(_x), y_vel(_y) {}
};

class ParticleColor {
public:
	uint8_t color_r = 255, color_g = 255, color_b = 255, color_a = 255;

	ParticleColor() {}

	ParticleColor(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) : color_r(_r), color_g(_g), color_b(_b), color_a(_a) {}
};

class ParticleSystem
{
public:
	std::string type = "ParticleSystem";
	std::string key = "???";
	Actor* actor = nullptr;
	bool enabled = true;
	bool need_reset = false;
	bool stopped = false;

	int local_frame_number = 0;

	float emit_angle_min = 0.0f;
	float emit_angle_max = 360.0f;
	float emit_radius_min = 0.0f;
	float emit_radius_max = 0.5f;
	float rotation_min = 0.0f;
	float rotation_max = 0.0f;
	float start_scale_min = 1.0f;
	float start_scale_max = 1.0f;
	float start_speed_min = 0.0f;
	float start_speed_max = 0.0f;
	float rotation_speed_min = 0.0f;
	float rotation_speed_max = 0.0f;

	RandomEngine emit_angle_distribution;
	RandomEngine emit_radius_distribution;
	RandomEngine rotation_distribution;
	RandomEngine scale_distribution;
	RandomEngine speed_distribution;
	RandomEngine rotation_speed_distribution;

	float x = 0.0f, y = 0.0f;
	int frames_between_bursts = 1;
	int burst_quantity = 1;
	uint8_t start_color_r = 255, start_color_g = 255, start_color_b = 255, start_color_a = 255;
	int end_color_r = -1, end_color_g = -1, end_color_b = -1, end_color_a = -1;
	std::string image = "";
	int sorting_order = 9999;
	int duration_frames = 300;
	float gravity_scale_x = 0.0f, gravity_scale_y = 0.0f;
	float drag_factor = 1.0f;
	float angular_drag_factor = 1.0f;
	float end_scale = -1;

	// index with the frame that the particle came into existence for the start of the indices that burst occupies
	std::queue<int> free_list;
	std::vector<int> start_frames;
	std::vector<ParticlePos> particle_positions;
	std::vector<ParticleVelocity> particle_velocities;
	std::vector<float> particle_initial_scales;
	std::vector<float> particle_scales;
	std::vector<float> particle_rotations;
	std::vector<float> particle_rotation_speeds;

	// Access the color that a particle should be on a given frame by the frames its been alive
	std::vector<ParticleColor> particle_colors;

	ParticleSystem();

	ParticleSystem(ParticleSystem* other);

	void OnStart();

	void OnUpdate();

	void GenerateNewParticles(int quantity);

	void UpdateParticles();

	void RemoveParticles(int starting_index);

	void CalculateAllColors();

	void Stop();

	void Play();

	void Burst();
};

#endif