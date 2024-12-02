#include "PhysicsObjectModifier.h"
#include "../../../PhysicsEngine/PhysicsEngine.h"
#include "../../Object3D.h"

PhysicsObjectModifier::PhysicsObjectModifier(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine) : Modifier(parent_game_object_3d)
{
	physics_engine->register_physics_object(this);
}

void PhysicsObjectModifier::calculate_forces()
{
	clear_force();
}


void PhysicsObjectModifier::clear_force()
{
	force_ = glm::vec3(0, 0, 0);
}

void PhysicsObjectModifier::add_force(const glm::vec3 force)
{
	this->force_ += force;
}


glm::vec3 PhysicsObjectModifier::get_velocity() const
{
	return velocity_;
}

void PhysicsObjectModifier::draw_gui()
{
	ImGui::SeparatorText("Physics Object");
	ImGui::Checkbox("Fix object", &is_fixed);
	ImGui::Checkbox("Gravity enabled", &gravity_enabled);
	ImGui::InputFloat("Mass",&this->mass);

}

void PhysicsObjectModifier::calculate_acceleration()
{
	acceleration_ = (force_/mass);
	if (gravity_enabled)
	{
		acceleration_+=physics_constants::gravity_vector;
	}
}

void PhysicsObjectModifier::integrate_velocity(integrator *integrator, const float delta)
{
	velocity_ = integrator->integrate(velocity_,acceleration_,delta);
}

void PhysicsObjectModifier::integrate_position(integrator* integrator, float delta) const
{
	auto pos_delta = integrator->integrate_delta_only(velocity_,delta);
	parent->move_global(pos_delta);
}


void PhysicsObjectModifier::move(glm::vec3 increment) const
{
	parent->move_global(increment);
}
