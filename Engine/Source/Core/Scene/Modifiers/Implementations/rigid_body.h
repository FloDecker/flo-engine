#pragma once
#include "physics_object_modifier.h"
#include "../modifier.h"
#include "../../Content/Mesh.h"

class rigid_body final : public physics_object_modifier
{

private:
	Mesh *mesh_;

	glm::vec3 center_of_mass_object_space_;
	glm::mat3 inertia_tensor_object_space_;
	glm::mat3 inverse_inertia_tensor_object_space_;
	
public:
	rigid_body(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine, Mesh *mesh)
		: physics_object_modifier(parent_game_object_3d, physics_engine)
	{
		mesh_ = mesh;
		center_of_mass_object_space_ = mesh_->get_center_of_mass();
		inertia_tensor_object_space_ = mesh_->get_inertia_tensor();
		inverse_inertia_tensor_object_space_ = glm::inverse(inertia_tensor_object_space_);
	}

	void calculate_forces() override;
	void draw_gui() override;
	~rigid_body() override;
};
