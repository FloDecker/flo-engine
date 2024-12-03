#include "physics_object_modifier.h"
#include "../../../PhysicsEngine/PhysicsEngine.h"
#include "../../Object3D.h"

physics_object_modifier::physics_object_modifier(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine) : modifier(parent_game_object_3d)
{
	physics_engine->register_physics_object(this);
}

void physics_object_modifier::calculate_forces()
{
	clear_force();
}


void physics_object_modifier::clear_force()
{
	force_ = glm::vec3(0, 0, 0);
}

void physics_object_modifier::add_force(const glm::vec3 force)
{
	this->force_ += force;
}


glm::vec3 physics_object_modifier::get_velocity() const
{
	return velocity_;
}

void physics_object_modifier::draw_gui()
{
	ImGui::SeparatorText("Physics Object");
	ImGui::Checkbox("Fix object", &is_fixed);
	ImGui::Checkbox("Gravity enabled", &gravity_enabled);
	ImGui::InputFloat("Mass",&this->mass);

}

void physics_object_modifier::calculate_acceleration()
{
	acceleration_ = (force_/mass);
	if (gravity_enabled)
	{
		acceleration_+=physics_constants::gravity_vector;
	}
}

void physics_object_modifier::integrate_velocity(integrator *integrator, const float delta)
{
	velocity_ = integrator->integrate(velocity_,acceleration_,delta);
}

void physics_object_modifier::integrate_position(integrator* integrator, float delta) const
{
	auto pos_delta = integrator->integrate_delta_only(velocity_,delta);
	parent->move_global(pos_delta);
}


void physics_object_modifier::move(glm::vec3 increment) const
{
	parent->move_global(increment);
}
