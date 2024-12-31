#pragma once
#include "Texture/Texture2D.h"


class framebuffer_object
{
public:
	framebuffer_object();
	void render_to_framebuffer() const;
	void attach_texture_as_depth_buffer(Texture2D* depth_texture);
	void attach_texture_as_color_buffer(Texture2D* color_texture);
private:
	void generate_framebuffer();
	static bool check_attached_framebuffer();

	bool has_color_attachment_ = false;
	bool has_depth_attachment_ = false;
	Texture2D* color_texture_ = nullptr;
	Texture2D* depth_texture_ = nullptr;
	unsigned int fbo_ = 0;
};
