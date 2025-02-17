#include "collider_modifier.h"
#include "../../../../PhysicsEngine/PhysicsEngine.h"
#include "../../../Object3D.h"

collider_modifier::collider_modifier(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine): modifier(parent_game_object_3d)
{
	physics_engine->register_collider(this);
	//search if the parent objects has rigid body modifier
	
	auto parent_collider = parent_game_object_3d->get_modifiers_by_id(10);
	for (auto modifier : parent_collider)
	{
		rigid_body* r = dynamic_cast<rigid_body*>(modifier);
		if (r == nullptr)
		{
			std::cerr << "tried casting modifier with id 10 but couldn't cast object to rigid body modifier\n";
		} else
		{
			r->colliders.insert(this);
			associated_rigid_body = r;
		}
	}	
}

int collider_modifier::get_id()
{
	return 100;
}

rigid_body* collider_modifier::get_associated_rigid_body() const
{
	if (associated_rigid_body == nullptr)
	{
		std::cerr << "no rigid body associated with this collider \n";
		return nullptr;
	}
	return associated_rigid_body;
}
