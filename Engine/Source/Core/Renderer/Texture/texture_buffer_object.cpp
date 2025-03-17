#include "texture_buffer_object.h"

#include <GL/glew.h>

texture_buffer_object::texture_buffer_object(int data_size)
{
	data_size_ = data_size;
}

texture_buffer_object::texture_buffer_object(int data_size, std::vector<float>* data): texture_buffer_object(data_size)
{
	init_(data);
}

void texture_buffer_object::init_(std::vector<float>* data)
{
	if (initialized_)
	{
		return;
	}

	if (data->size() < data_size_)
	{
		data->resize(data_size_, 0.0f);
	}


	// Step 1: Generate and bind the buffer
	glGenBuffers(1, &texture_buffer_);
	glBindBuffer(GL_TEXTURE_BUFFER, texture_buffer_);

	// Step 2: Allocate and fill buffer with data
	glBufferData(GL_TEXTURE_BUFFER, data_size_ * sizeof(float), data->data(), GL_STATIC_DRAW);

	// Step 3: Generate a texture and bind it to the buffer
	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_BUFFER, texture_);

	// Step 4: Attach the buffer to the texture
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, texture_buffer_);

	// Cleanup: Unbind
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glBindTexture(GL_TEXTURE_BUFFER, 0);

	initialized_ = true;
}
