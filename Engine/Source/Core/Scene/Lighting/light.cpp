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

void light::draw_object_specific_ui()
{
	ImGui::Text("Light settings");
	bool changed = false;
	changed = changed || ImGui::ColorPicker3("Light Color", &color[0]);
	changed = changed || ImGui::InputFloat("Light intensity", &intensity);
	if (changed) on_light_changed();
}

void light::on_light_changed()
{
}

void light::on_transform_changed()
{
	on_light_changed();
}
