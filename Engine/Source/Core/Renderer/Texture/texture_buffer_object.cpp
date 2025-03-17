#include "texture_buffer_object.h"

#include <GL/glew.h>

texture_buffer_object::texture_buffer_object(int data_size)
{
	data_size_ = data_size;
}

texture_buffer_object::texture_buffer_object(int data_size, std::vector<float>* data): texture_buffer_object(data_size)
{
	init_float_(data);
}

void texture_buffer_object::generate_and_bind_buffer()
{
	// Step 1: Generate and bind the buffer
	glGenBuffers(1, &texture_buffer_);
	glBindBuffer(GL_TEXTURE_BUFFER, texture_buffer_);
}

void texture_buffer_object::generate_and_attach_texture()
{
	// Step 3: Generate a texture and bind it to the buffer
	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_BUFFER, texture_);

	// Step 4: Attach the buffer to the texture
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, texture_buffer_);

	// Cleanup: Unbind
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

bool texture_buffer_object::init(int data_size, std::vector<float>* data)
{
	data_size_ = data_size;
	if (initialized_) {return false;}
	init_float_(data);
	return true;
}

bool texture_buffer_object::init(int data_size, std::vector<glm::vec3>* data)
{
	data_size_ = data_size;
	if (initialized_) {return false;}
	init_vec3_(data);
	return true;
}

void texture_buffer_object::init_float_(std::vector<float>* data)
{
	if (data->size() < data_size_) { data->resize(data_size_, 0.0f); }
	generate_and_bind_buffer();
	// Step 2: Allocate and fill buffer with data
	glBufferData(GL_TEXTURE_BUFFER, data_size_ * sizeof(float), data->data(), GL_STATIC_DRAW);
	generate_and_attach_texture();
	initialized_ = true;
}


void texture_buffer_object::init_vec3_(std::vector<glm::vec3>* data)
{
	if (data->size() < data_size_) { data->resize(data_size_, glm::vec3(0.0f)); }
	generate_and_bind_buffer();
	// Step 2: Allocate and fill buffer with data
	glBufferData(GL_TEXTURE_BUFFER, data_size_ * sizeof(glm::vec3), data->data(), GL_STATIC_DRAW);
	generate_and_attach_texture();
	initialized_ = true;
}
