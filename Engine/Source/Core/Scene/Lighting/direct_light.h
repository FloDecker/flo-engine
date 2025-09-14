#pragma once
#include "light.h"
#include "../../Renderer/Texture/texture_2d.h"
#include "../../Renderer/framebuffer_object.h"


struct ubo_direct_light_data
{
	alignas(16) glm::vec3 light_direction;
	alignas(4) float light_intensity;
	alignas(16)glm::vec3 light_color;
	alignas(4) float light_angle;
	alignas(16) glm::mat4 direct_light_light_space_matrix;

};

class Line3D;

class direct_light : public light
{
	float size_;
	float light_height_; //distance of sun to the camera
	float near_plane_;
	float far_plane_;
	texture_2d* light_map_;
	framebuffer_object* light_map_fbo_;

	//this is the position the light uses as center during the light pass
	glm::vec3 light_center_position_ = {0, 0, 0};

	Line3D* debug_line_;

public:
	explicit direct_light(Object3D* parent, unsigned int light_map_width, unsigned int light_map_height);
	void set_light_settings(float size, float near_plane, float far_plane, float light_camera_distance);
	void render_to_light_map() override;
	float angle = 10.0f;
	float light_pos_update_interval = 1.0f;
	void draw_object_specific_ui() override;
	glm::vec3 get_light_direction();

	//this should be before the light pass to update the lights position
	void set_light_center_position(glm::vec3 position);

	[[nodiscard]] texture_2d* light_map() const
	{
		return light_map_;
	}

private:
	void on_light_changed() override;
	uniform_buffer_object<ubo_direct_light_data> direct_light_ubo_;
};
