#include "modifier.h"
#include "../Object3D.h"

modifier::modifier(Object3D* parent_game_object_3d)
{
	this->parent = parent_game_object_3d;
}

void modifier::draw_gui()
{
}

int modifier::get_id()
{
	return -1;
}

Object3D* modifier::get_parent() const
{
	return this->parent;
}
