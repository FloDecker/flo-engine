#include "PhysicsEngine.h"

#include "IntegrationMethods/Integrator.h"

void PhysicsEngine::evaluate_physics_step(double delta_t)
{
	for (auto physics_object : physics_objects_)
	{
		physics_object->calculate_forces();
		physics_object->integrate_velocity(this->integrator_euler,delta_t);
		physics_object->integrate_position(this->integrator_euler,delta_t);
	}
}

void PhysicsEngine::register_physics_object(PhysicsObjectModifier* object)
{
	physics_objects_.push_back(object);
}
