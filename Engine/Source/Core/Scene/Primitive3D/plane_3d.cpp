#include "plane_3d.h"

#include "../Scene.h"
#include "../Modifiers/Implementations/Colliders/box_collider.h"

plane_3d::plane_3d(Object3D* parent): a_single_primitive(
	parent, new primitive_instance(this, parent->get_scene()->get_global_context()->global_primitives.plane))
{
	this->name = "Plane";
	add_modifier(new box_collider(this, {0.5, 0.5, 0.01}, {-0.5, -0.5, -0.01}));
}
