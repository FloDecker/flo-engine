#include "PhysicsObjectModifier.h"
#include "../../../PhysicsEngine/PhysicsEngine.h"
#include "../../Object3D.h"

PhysicsObjectModifier::PhysicsObjectModifier(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine) : Modifier(parent_game_object_3d)
{
	physics_engine->register_physics_object(this);
}

void PhysicsObjectModifier::move(glm::vec3 increment) const
{
	parent->move_global(increment);
}
