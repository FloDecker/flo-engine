#pragma once
#include <unordered_set>

#include "physics_object_modifier.h"
#include "../modifier.h"
#include "../../Content/Mesh.h"

class collider_modifier;

class rigid_body final : public physics_object_modifier
{
	std::vector<glm::vec3> distances_to_center_of_mass_; //x_i
	std::vector<glm::vec3> force_on_vertices_; //f_i

	glm::vec3 pos_center_of_mass_object_space_initial_;


	glm::vec3 pos_center_of_mass_object_space_; //x_cm
	glm::vec3 velocity_center_of_mass_object_space_; //v_cm
	glm::mat3 inverse_inertia_tensor_object_space_; //I_0^-1
	glm::mat3 inverse_inertia_tensor_world_space_; //I^-1

	glm::vec3 angular_momentum_; //L
	glm::vec3 angular_velocity_; //w
	glm::vec3 torque_; //q


	void calculate_torque_and_force();

	void update_inverse_inertia_tensor_world_space();
	void update_angular_velocity();

public:
	rigid_body(Object3D* parent_game_object_3d);
	int get_id() override;

	void step(float delta);
	void apply_force_at_vertex(unsigned int vertex_id, glm::vec3 force);
	void apply_force_ws(glm::vec3 direction_ws, glm::vec3 pos_ws, float force);

	void recalculate_inertia(unsigned int index_of_collider = 0);

	glm::vec3 current_linear_impulse;
	glm::vec3 current_angular_impulse;

	void clear_force() override;
	void clear_impulses();

	bool skip_next_step = false;

	//setters
	//TODO: this shouldnt be overwritten center of mass velocity = object velocity 
	void set_velocity(glm::vec3 velocity);
	void set_angular_momentum(glm::vec3 angular_momentum);

	//getters
	glm::vec3 get_angular_velocity() const;
	glm::mat3 get_inverse_inertia_tensor_world_space() const;
	glm::vec3 get_velocity() const;


	collider_modifier* collider;
	//colliders associated with this rigid body
	//TODO : one rigid body should be able to have more colliders
	//std::unordered_set<collider_modifier*> colliders;

	//physical characteristics
	float bounciness = 0.2f;
};
