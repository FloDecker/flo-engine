#include "Modifier.h"
#include "../Object3D.h"

Modifier::Modifier(Object3D* parent_game_object_3d)
{
	this->parent = parent_game_object_3d;
}

Object3D* Modifier::get_parent() const
{
	return this->parent;
}
