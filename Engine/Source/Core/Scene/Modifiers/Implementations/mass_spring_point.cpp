#include "mass_spring_point.h"

mass_spring_point::mass_spring_point(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine):
	PhysicsObjectModifier(parent_game_object_3d, physics_engine)
{
}

void mass_spring_point::calculate_forces()
{
	add_force(-get_velocity()*damp);
}

