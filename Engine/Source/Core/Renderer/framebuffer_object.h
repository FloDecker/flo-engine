#pragma once
#include "Texture/texture_2d.h"


class framebuffer_object
{
public:
	framebuffer_object();
	void render_to_framebuffer() const;
	void attach_texture_as_depth_buffer(texture_2d* depth_texture);
	void attach_texture_as_color_buffer(texture_2d* color_texture, unsigned int attachment_point);
	void resize_attach_textures(unsigned int width, unsigned int height) const;
	texture_2d* get_color_attachment_at_index(unsigned int index) const;

private:
	void generate_framebuffer();
	static bool check_attached_framebuffer();

	bool has_depth_attachment_ = false;
	texture_2d* depth_texture_ = nullptr;
	unsigned int fbo_ = 0;
	std::vector<texture_2d*> color_attachments_;
};
