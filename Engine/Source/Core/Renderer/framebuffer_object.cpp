#include "framebuffer_object.h"

#include <vec3.hpp>
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

int framebuffer_object::read_pixel_as_integer(int x, int y) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

	GLint pixelColor; // To store the RGBA color
	glReadPixels(x, y, 1, 1, GL_RED, GL_INT, &pixelColor);
	return pixelColor;
}

glm::vec3 framebuffer_object::read_pixel_as_rgb(int x, int y) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

	glm::vec3 pixelColor; // To store the RGBA color
	glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, &pixelColor);
	return pixelColor;
}



void framebuffer_object::generate_framebuffer()
{
	glGenFramebuffers(1, &fbo_);
}

bool framebuffer_object::check_attached_framebuffer()
{
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch (status) {
		case GL_FRAMEBUFFER_UNDEFINED:
			std::cerr << "Framebuffer undefined. No default framebuffer exists." << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			std::cerr << "Framebuffer incomplete: Attachment issue." << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			std::cerr << "Framebuffer incomplete: No attachments found." << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			std::cerr << "Framebuffer incomplete: Draw buffer is not properly set." << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			std::cerr << "Framebuffer incomplete: Read buffer is not properly set." << std::endl;
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			std::cerr << "Framebuffer unsupported: Combination of formats not supported." << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			std::cerr << "Framebuffer incomplete: Inconsistent number of samples." << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			std::cerr << "Framebuffer incomplete: Layer targets are incorrect." << std::endl;
			break;
		default:
			std::cerr << "Unknown framebuffer status: " << status << std::endl;
			break;
		}
		return false;


	}

	
	return true;
}
