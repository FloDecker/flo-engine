#pragma once
#include "physics_object_modifier.h"
#include "../modifier.h"
#include "../../Content/Mesh.h"

class MeshCollider;

class rigid_body final : public physics_object_modifier
{

private:
	MeshCollider *collider_;

	std::vector<glm::vec3> distances_to_center_of_mass_;	//x_i
	std::vector<glm::vec3> force_on_vertices_;				//f_i

	glm::vec3 pos_center_of_mass_object_space_initial_;


	glm::vec3 pos_center_of_mass_object_space_;			    //x_cm
	glm::vec3 velocity_center_of_mass_object_space_;	    //v_cm
	glm::mat3 inverse_inertia_tensor_object_space_;		    //I_0^-1
	glm::mat3 inverse_inertia_tensor_world_space_;		    //I^-1

	glm::vec3 angular_momentum_; 						    //L
	glm::vec3 angular_velocity_; 						    //w
	glm::vec3 torque_;										//q


	void calculate_torque_and_force();						
	
	void update_inverse_inertia_tensor_world_space();
	void update_angular_velocity();
	
public:
	rigid_body(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine, MeshCollider *collider);
	

	void step(float delta);
	void apply_force_at_vertex(unsigned int vertex_id, glm::vec3 force);
	void apply_cylindircal_force_ws(glm::vec3 direction, float force);
	void clear_force() override;
};
