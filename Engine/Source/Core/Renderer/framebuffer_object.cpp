#include "framebuffer_object.h"

#include <GL/glew.h>


framebuffer_object::framebuffer_object() {
	generate_framebuffer();
}

void framebuffer_object::render_to_framebuffer() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
	if (has_color_attachment_)
	{
		glViewport(0, 0, color_texture_->width(), color_texture_->height());
	}

	if (has_depth_attachment_)
	{
		glViewport(0, 0, depth_texture_->width(), depth_texture_->height());
	}
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear color buffer
}

void framebuffer_object::attach_texture_as_depth_buffer(Texture2D *depth_texture)
{
	depth_texture_ = depth_texture;
	has_depth_attachment_ = true;
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture->get_texture(), 0);
	//glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);
	check_attached_framebuffer();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer_object::attach_texture_as_color_buffer(Texture2D *color_texture)
{
	color_texture_ = color_texture;
	has_color_attachment_ = true;
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture->get_texture(), 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	check_attached_framebuffer();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer_object::resize_attach_textures(unsigned int width, unsigned int height) const
{
	if (has_color_attachment_)
	{
		color_texture_->resize(width, height);
	}
	if (has_depth_attachment_)
	{
		depth_texture_->resize(width, height);
	}
}


void framebuffer_object::generate_framebuffer()
{
	glGenFramebuffers(1, &fbo_);
}

bool framebuffer_object::check_attached_framebuffer()
{
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR: Framebuffer is not complete!\n";
		return false;
	}
	return true;
}
