#pragma once
#include "light.h"
#include "../../Renderer/Texture/Texture2D.h"
#include "../../Renderer/framebuffer_object.h"

class Line3D;

class direct_light : public light
{
private:
	float size_;
	float light_height_ = 10; //distance of sun to the camera
	float near_plane_;
	float far_plane_;
	Texture2D *light_map_;
	framebuffer_object *light_map_fbo_;

	//this is the position the light uses as center during the light pass
	glm::vec3 light_center_position_ = {0,0,0};

	Line3D *debug_line_;
public:
	explicit direct_light(Object3D* parent, unsigned int light_map_width,  unsigned int light_map_height );
	void set_light_settings(float size, float near_plane, float far_plane);
	void render_to_light_map() override;
	float angle = 10.0f;
	float light_pos_update_interval = 1.0f;
	void draw_object_specific_ui() override;
	glm::vec3 get_light_direction();

protected:
	void on_light_changed() override;

public:
	//this should be before the light pass to update the lights position
	void set_light_center_position(glm::vec3 position);

	[[nodiscard]] Texture2D* light_map() const
	{
		return light_map_;
	}
};
