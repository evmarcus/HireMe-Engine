#include "ParticleSystem.h"
#include "ImageDB.h"
#include "renderer.h"

ParticleSystem::ParticleSystem() {

}

ParticleSystem::ParticleSystem(ParticleSystem* other) {
	emit_angle_min = other->emit_angle_min;
	emit_angle_max = other->emit_angle_max;
	emit_radius_min = other->emit_radius_min;
	emit_radius_max = other->emit_radius_max;
	rotation_min = other->rotation_min;
	rotation_max = other->rotation_max;
	start_scale_min = other->start_scale_min;
	start_scale_max = other->start_scale_max;
	start_speed_min = other->start_speed_min;
	start_speed_max = other->start_speed_max;
	rotation_speed_min = other->rotation_speed_min;
	rotation_speed_max = other->rotation_speed_max;
	x = other->x;
	y = other->y;
	frames_between_bursts = other->frames_between_bursts;
	burst_quantity = other->burst_quantity;
	start_color_r = other->start_color_r;
	start_color_g = other->start_color_g;
	start_color_b = other->start_color_b;
	start_color_a = other->start_color_a;
	end_color_r = other->end_color_r;
	end_color_g = other->end_color_g;
	end_color_b = other->end_color_b;
	end_color_a = other->end_color_a;
	image = other->image;
	sorting_order = other->sorting_order;
	duration_frames = other->duration_frames;
	gravity_scale_x = other->gravity_scale_x;
	gravity_scale_y = other->gravity_scale_y;
	drag_factor = other->drag_factor;
	angular_drag_factor = other->angular_drag_factor;
	end_scale = other->end_scale;
}

void ParticleSystem::OnStart() {
	emit_angle_distribution = RandomEngine(emit_angle_min, emit_angle_max, 298);
	emit_radius_distribution = RandomEngine(emit_radius_min, emit_radius_max, 404);
	rotation_distribution = RandomEngine(rotation_min, rotation_max, 440);
	scale_distribution = RandomEngine(start_scale_min, start_scale_max, 494);
	speed_distribution = RandomEngine(start_speed_min, start_speed_max, 498);
	rotation_speed_distribution = RandomEngine(rotation_speed_min, rotation_speed_max, 305);

	if (image == "") ImageDB::CreateDefaultParticleTextureWithName(image);
	if (frames_between_bursts < 1) frames_between_bursts = 1;
	if (burst_quantity < 1) burst_quantity = 1;
	if (duration_frames < 1) duration_frames = 1;

	// This is the maximum particle density if particles disappear at a higher rate than they spawn
	int max_particle_density = ceil(duration_frames / frames_between_bursts) * burst_quantity;

	start_frames.reserve(max_particle_density);
	particle_positions.reserve(max_particle_density);
	particle_initial_scales.reserve(max_particle_density);
	particle_scales.reserve(max_particle_density);
	particle_rotations.reserve(max_particle_density);
	particle_velocities.reserve(max_particle_density);
	particle_rotation_speeds.reserve(max_particle_density);
	
	particle_colors.resize(duration_frames);
	CalculateAllColors();
}

void ParticleSystem::OnUpdate() {
	if (!enabled) 
		return;

	if (!stopped && local_frame_number % frames_between_bursts == 0) {
		GenerateNewParticles(burst_quantity);
	}

	UpdateParticles();

	for (int i = 0; i < start_frames.size(); i++) {
		if (start_frames[i] == -1) continue;

		int age = local_frame_number - start_frames[i];
		if (age >= duration_frames) {
			RemoveParticles(i);
			i += burst_quantity - 1;
			continue;
		}

		Renderer::DrawParticleEx(image, particle_positions[i].x, particle_positions[i].y,
			particle_rotations[i], particle_scales[i], particle_scales[i], 0.5f, 0.5f, 
			particle_colors[age].color_r, particle_colors[age].color_g, 
			particle_colors[age].color_b, particle_colors[age].color_a, sorting_order);
	}

	local_frame_number++;
}

void ParticleSystem::GenerateNewParticles(int quantity) {

	int start_index = start_frames.size();
	
	// If there is an existing index waiting in the queue, use that
	if (!free_list.empty()) {
		start_index = free_list.front();
		free_list.pop();
		for (int i = 0; i < quantity; i++) {
			start_frames[start_index + i] = local_frame_number;
		}
	}
	else {
		particle_positions.resize(particle_positions.size() + quantity);
		particle_initial_scales.resize(particle_initial_scales.size() + quantity);
		particle_scales.resize(particle_scales.size() + quantity);
		particle_rotations.resize(particle_rotations.size() + quantity);
		particle_velocities.resize(particle_velocities.size() + quantity);
		particle_rotation_speeds.resize(particle_rotation_speeds.size() + quantity);
		for (int i = 0; i < quantity; i++) {
			start_frames.emplace_back(local_frame_number);
		}
	}

	//existing_particles[local_frame_number] = start_index;

	for (int i = 0; i < quantity; i++) {
		float angle_radians = glm::radians(emit_angle_distribution.Sample());
		float radius = emit_radius_distribution.Sample();

		float cos_angle = glm::cos(angle_radians);
		float sin_angle = glm::sin(angle_radians);

		particle_positions[start_index + i] = { x + cos_angle * radius, y +  sin_angle * radius };

		float speed = speed_distribution.Sample();

		particle_velocities[start_index + i] = { cos_angle * speed, sin_angle * speed };
	}

	for (int i = 0; i < quantity; i++) {
		particle_initial_scales[start_index + i] = scale_distribution.Sample();
		particle_scales[start_index + i] = particle_initial_scales[start_index + i];
	}

	for (int i = 0; i < quantity; i++) {
		particle_rotations[start_index + i] = rotation_distribution.Sample();
	}

	for (int i = 0; i < quantity; i++) {
		particle_rotation_speeds[start_index + i] = rotation_speed_distribution.Sample();
	}
}

void ParticleSystem::UpdateParticles() {
	for (int i = 0; i < start_frames.size(); i++) {
		if (start_frames[i] == -1) continue;

		particle_velocities[i].x_vel += gravity_scale_x;
		particle_velocities[i].x_vel *= drag_factor;
		particle_velocities[i].y_vel += gravity_scale_y;
		particle_velocities[i].y_vel *= drag_factor;

		particle_positions[i] = { particle_positions[i].x + particle_velocities[i].x_vel,
			particle_positions[i].y + particle_velocities[i].y_vel };
	}
	if (rotation_speed_min != 0.0f || rotation_speed_max != 0.0f || angular_drag_factor != 1.0f) {
		for (int i = 0; i < start_frames.size(); i++) {
			if (start_frames[i] == -1) continue;

			particle_rotation_speeds[i] *= angular_drag_factor;

			particle_rotations[i] = particle_rotations[i] + particle_rotation_speeds[i];
		}
	}
	/*if (gravity_scale_x != 0.0f || gravity_scale_y != 0.0f) {
		for (int i = 0; i < start_frames.size(); i++) {
			if (start_frames[i] == -1) continue;

			particle_rotations[i] = particle_rotations[i] + particle_rotation_speeds[i];
		}
	}*/
	if (end_scale != -1) {
		for (int i = 0; i < start_frames.size(); i++) {
			if (start_frames[i] == -1) continue;

			float lifetime_progress = static_cast<float>(local_frame_number - start_frames[i]) / duration_frames;

			particle_scales[i] = glm::mix(particle_initial_scales[i], end_scale, lifetime_progress);
		}
	}
}

void ParticleSystem::RemoveParticles(int starting_index) {
	for (int i = starting_index; i < starting_index + burst_quantity; i++) {
		start_frames[i] = -1;
	}

	free_list.push(starting_index);
}

void ParticleSystem::CalculateAllColors() {
	if (end_color_r == -1 && end_color_g == -1 && end_color_b == -1 && end_color_a == -1) {
		for (int i = 0; i < duration_frames; i++) {
			float lifetime_progress = static_cast<float>(i) / duration_frames;

			particle_colors[i] = {
				start_color_r,
				start_color_g,
				start_color_b,
				start_color_a
			};
		}

		return;
	}

	if (end_color_r == -1) end_color_r = start_color_r;
	if (end_color_g == -1) end_color_g = start_color_g;
	if (end_color_b == -1) end_color_b = start_color_b;
	if (end_color_a == -1) end_color_a = start_color_a;

	for (int i = 0; i < duration_frames; i++) {
		float lifetime_progress = static_cast<float>(i) / duration_frames;

		particle_colors[i] = {
			(start_color_r == end_color_r) ? start_color_r : glm::mix(start_color_r, static_cast<uint8_t>(end_color_r), lifetime_progress),
			(start_color_g == end_color_g) ? start_color_g : glm::mix(start_color_g, static_cast<uint8_t>(end_color_g), lifetime_progress),
			(start_color_b == end_color_b) ? start_color_b : glm::mix(start_color_b, static_cast<uint8_t>(end_color_b), lifetime_progress),
			(start_color_a == end_color_a) ? start_color_a : glm::mix(start_color_a, static_cast<uint8_t>(end_color_a), lifetime_progress)
		};
	}
}

void ParticleSystem::Stop() {
	stopped = true;
}

void ParticleSystem::Play() {
	stopped = false;
}

void ParticleSystem::Burst() {
	GenerateNewParticles(burst_quantity);
}
