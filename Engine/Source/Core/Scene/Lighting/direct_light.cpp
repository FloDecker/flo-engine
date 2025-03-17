#include "direct_light.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/string_cast.hpp>

#include "../Scene.h"
#include "../DebugPrimitives/Line3D.h"

direct_light::direct_light(Object3D* parent, unsigned int light_map_width, unsigned int light_map_height): light(parent)
{
	scene_->register_global_light(this);
	set_light_settings(10, 0.1, 100);
	light_map_ = new texture_2d();
	light_map_->initialize_as_depth_map_render_target(light_map_width, light_map_height);
	light_map_fbo_ = new framebuffer_object();
	light_map_fbo_->attach_texture_as_depth_buffer(light_map_);
	debug_line_ = new Line3D(this, glm::vec3(0.0), -vec_z * 20.0f);
	debug_line_->color = glm::vec3(1, 224. / 255., 102. / 255.);
	this->name = "direct_light";
}

void direct_light::set_light_settings(float size, float near_plane, float far_plane)
{
	size_ = size;
	near_plane_ = near_plane;
	far_plane_ = far_plane;
	on_light_changed();
}

void direct_light::render_to_light_map()
{
	light_map_fbo_->render_to_framebuffer();
}

void direct_light::draw_object_specific_ui()
{
	light::draw_object_specific_ui();
	ImGui::DragFloat("Size", &size_);
	ImGui::DragFloat("Light to camera distance", &light_height_);
}

glm::vec3 direct_light::get_light_direction()
{
	return getForwardVector();
}


void direct_light::set_light_center_position(glm::vec3 position)
{
	if (distance(position, light_center_position_) > light_pos_update_interval)
	{
		light_center_position_ = position;
		on_light_changed();
	}
}

void direct_light::on_light_changed()
{
	glm::mat4 light_projection = glm::ortho(-size_, size_, -size_, size_, near_plane_, far_plane_);
	auto light_pos = glm::mat4(1.0f);
	light_pos = translate(light_pos, light_center_position_ + getForwardVector() * light_height_);
	light_pos = light_pos * toMat4(this->get_quaternion_rotation());

	light_matrix_ = light_projection * inverse(light_pos);

	auto l = global_context_->uniform_buffer_object->ubo_direct_light;
	l->light_direction = getForwardVector();
	l->light_intensity = intensity;
	l->light_color = color;
	l->light_angle = angle;
	l->direct_light_light_space_matrix = light_matrix_;

	global_context_->uniform_buffer_object->update_direct_light();
}
