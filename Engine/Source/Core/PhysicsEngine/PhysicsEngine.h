#pragma once
#include <vector>

#include "../Scene/Modifiers/Implementations/PhysicsObjectModifier.h"

class PhysicsEngine
{
public:
	void evaluate_physics_step(double delta_t);
	void register_physics_object(PhysicsObjectModifier* object);
private:
	std::vector<PhysicsObjectModifier*> physics_objects_;
	
};
