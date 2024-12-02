#include "PhysicsEngine.h"

#include "IntegrationMethods/Integrator.h"
#define GRAVITY_FORCE (-9.81f)
#define GRAVITY_VECTOR (glm::vec3(0.0f, GRAVITY_FORCE, 0.0f))

void PhysicsEngine::evaluate_physics_step(double delta_t)
{
	for (auto physics_object : physics_objects_)
	{
		physics_object->acceleration_ = GRAVITY_VECTOR;
		physics_object-> velocity_ = Integrator::integrate_euler(physics_object-> velocity_, physics_object->acceleration_, delta_t);
		physics_object->move(physics_object-> velocity_ * static_cast<float>(delta_t));
	}
}

void PhysicsEngine::register_physics_object(PhysicsObjectModifier* object)
{
	physics_objects_.push_back(object);
}
