#pragma once

#include <vector>

#include "texture.h"

class texture_buffer_object : public texture
{
public:
	texture_buffer_object(int data_size);
	texture_buffer_object(int data_size, std::vector<float>* data);

private:
	unsigned int data_size_;

	void init_(std::vector<float>* data);


	unsigned int texture_buffer_;
};
