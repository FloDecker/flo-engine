#pragma once
#include "light.h"
#include "../../Renderer/Texture/Texture2D.h"
#include "../../Renderer/framebuffer_object.h"

class direct_light : public light
{
private:
	float size_;
	float near_plane_;
	float far_plane_;
	Texture2D *light_map_;
	framebuffer_object *light_map_fbo_;
	
public:
	explicit direct_light(Object3D* parent, unsigned int light_map_width,  unsigned int light_map_height );
	void set_light_settings(float size, float near_plane, float far_plane);
	void render_to_light_map() override;
	float angle = 10.0f;
	void draw_object_specific_ui() override;

	[[nodiscard]] Texture2D* light_map() const
	{
		return light_map_;
	}

protected:
	void on_light_changed() override;
};
