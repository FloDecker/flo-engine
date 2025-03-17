#pragma once
#include <string>
#include <iostream>

#include "texture.h"

enum texture_type
{
	IMAGE_TEXTURE,
	FRAME_BUFFER_COLOR,
	FRAME_BUFFER_DEPTH,
};

class texture_2d : public texture
{
	std::string* _path;
	void initialize_from_data(unsigned char* data);

	//dimentions
	int width_ = 0;
	int height_ = 0;
	int _channleAmount = 0;

	texture_type type_;

public:
	void initialize_as_depth_map_render_target(unsigned int width, unsigned int height);
	void initialize_as_frame_buffer(unsigned int width, unsigned int height);
	unsigned int get_texture() const;
	void loadFromDisk(std::string* path);
	void resize(unsigned int width, unsigned int height);
	void generate_mip_map() const;

	[[nodiscard]] int width() const
	{
		return width_;
	}

	[[nodiscard]] int height() const
	{
		return height_;
	}
};
