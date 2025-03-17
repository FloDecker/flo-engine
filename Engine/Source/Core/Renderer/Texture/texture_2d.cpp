#include "texture_2d.h"
#include <GL/glew.h>
#include <stb_image.h>

void texture_2d::initialize_from_data(unsigned char* data)
{
	if (initialized_)
	{
		std::cerr << "Texture2D already initialized\n";
	}
	if (width_ == 0 || height_ == 0)
	{
		std::cerr << "Texture2D width or height cannot be zero\n";
	}
	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_2D, texture_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA8, width_, height_, 0,GL_RGBA,GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	type_ = IMAGE_TEXTURE;
	initialized_ = true;
}

void texture_2d::initialize_as_depth_map_render_target(const unsigned int width, const unsigned int height)
{
	if (initialized_)
	{
		std::cerr << "Texture2D already initialized\n";
	}
	width_ = width;
	height_ = height;

	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_2D, texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
	             width_, height_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	type_ = FRAME_BUFFER_DEPTH;
	initialized_ = true;
}

void texture_2d::initialize_as_frame_buffer(unsigned int width, unsigned int height)
{
	if (initialized_)
	{
		std::cerr << "Texture2D already initialized\n";
	}
	width_ = width;
	height_ = height;

	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_2D, texture_);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	type_ = FRAME_BUFFER_COLOR;

	initialized_ = true;
}


unsigned texture_2d::get_texture() const
{
	return texture_;
}

void texture_2d::loadFromDisk(std::string* path)
{
	this->_path = path;
	unsigned char* data = stbi_load(_path->c_str(), &width_, &height_, &_channleAmount, 0);
	if (!data)
	{
		std::cerr << "couldn't load image " << _path << std::endl;
	}
	this->initialize_from_data(data);
	stbi_image_free(data);
}

void texture_2d::resize(unsigned int width, unsigned int height)
{
	switch (type_)
	{
	case FRAME_BUFFER_DEPTH:
		width_ = width;
		height_ = height;
		glBindTexture(GL_TEXTURE_2D, texture_);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		             width_, height_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		break;
	case FRAME_BUFFER_COLOR:
		width_ = width;
		height_ = height;
		glBindTexture(GL_TEXTURE_2D, texture_);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		break;
	case IMAGE_TEXTURE:
		std::cerr << "Can't resize texture\n";
	}
}

void texture_2d::generate_mip_map() const
{
	glBindTexture(GL_TEXTURE_2D, texture_);
	glGenerateMipmap(GL_TEXTURE_2D);
}
