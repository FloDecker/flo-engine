#include "collider_modifier.h"
#include "../../../../PhysicsEngine/PhysicsEngine.h"

collider_modifier::collider_modifier(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine): physics_object_modifier(parent_game_object_3d,physics_engine)
{
	physics_engine->register_collider(this);
}
