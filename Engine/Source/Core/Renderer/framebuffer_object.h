#pragma once
#include "Texture/Texture2D.h"

enum attachment_mode
{
	ATTACH_AS_DEPTH_BUFFER,
};

class framebuffer_object
{
public:
	framebuffer_object(Texture2D* attached_texture, attachment_mode attachment_mode);
	void render_to_framebuffer() const;
	
private:
	void generate_framebuffer();
	
	void attach_texture_as_depth_buffer();
	unsigned int fbo_ = 0;
	Texture2D* attached_texture_ = nullptr;
};
