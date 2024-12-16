#include "rigid_body.h"

#include "../../Object3D.h"


void rigid_body::update_inverse_inertia_tensor_world_space()
{
	auto p = get_parent()->get_quaternion_rotation();
}
