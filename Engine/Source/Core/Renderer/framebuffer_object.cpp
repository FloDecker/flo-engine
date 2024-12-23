#include "framebuffer_object.h"

#include <GL/glew.h>


framebuffer_object::framebuffer_object(Texture2D* attached_texture, attachment_mode attachment_mode)
{
	attached_texture_ = attached_texture;
	generate_framebuffer();
	switch (attachment_mode)
	{
	case ATTACH_AS_DEPTH_BUFFER:
		attach_texture_as_depth_buffer();
		break;
	default: ;
	}
}

void framebuffer_object::render_to_framebuffer() const
{
	glViewport(0, 0, attached_texture_->width(), attached_texture_->height());
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void framebuffer_object::attach_texture_as_depth_buffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, attached_texture_->get_texture(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer_object::generate_framebuffer()
{
	glGenFramebuffers(1, &fbo_);
}
