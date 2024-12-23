#include "direct_light.h"

#include "../Scene.h"

direct_light::direct_light(Object3D* parent, unsigned int light_map_width, unsigned int light_map_height): light(parent)
{
	scene_->register_global_light(this);
	set_light_settings(10, 0.1, 100);
	light_map_ = new Texture2D();
	light_map_->initialize_as_depth_map_render_target(light_map_width, light_map_height);
	light_map_fbo_ = new framebuffer_object(light_map_, ATTACH_AS_DEPTH_BUFFER);
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

void direct_light::on_light_changed()
{
	glm::mat4 light_projection = glm::ortho(-size_, size_, -size_, size_, near_plane_, far_plane_);
	auto light_pos = glm::toMat4(this->get_quaternion_rotation());
	light_matrix_ = light_projection * light_pos;
}
