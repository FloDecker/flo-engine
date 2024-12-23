#include "light.h"

light::light(Object3D* parent) : Object3D(parent)
{
}

glm::mat4 light::get_light_space_matrix()
{
	return light_matrix_;
}

void light::render_to_light_map()
{
}

void light::on_light_changed()
{
}

void light::on_transform_changed()
{
	on_light_changed();
}
