#include "texture_buffer_object.h"

#include <iostream>
#include <GL/glew.h>

void texture_buffer_object::generate_and_bind_buffer()
{
	// Step 1: Generate and bind the buffer
	glGenBuffers(1, &texture_buffer_);
	glBindBuffer(GL_TEXTURE_BUFFER, texture_buffer_);
}

void texture_buffer_object::generate_and_attach_texture(int internal_format)
{
	// Step 3: Generate a texture and bind it to the buffer
	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_BUFFER, texture_);

	// Step 4: Attach the buffer to the texture
	glTexBuffer(GL_TEXTURE_BUFFER, internal_format, texture_buffer_);

	// Cleanup: Unbind
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

bool texture_buffer_object::init_float(unsigned int data_size)
{
	data_size_ = data_size;
	if (initialized_) { return false; }
	generate_and_bind_buffer();
	// Step 2: Allocate and fill buffer with data
	std::vector<float> data(data_size_, 0);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(float) * data_size_, data.data(), GL_STATIC_DRAW);
	generate_and_attach_texture(GL_R32F);
	initialized_ = true;
}

bool texture_buffer_object::init_vec3(unsigned int data_size)
{
	data_size_ = data_size;
	if (initialized_) { return false; }
	generate_and_bind_buffer();
	// Step 2: Allocate and fill buffer with data
	std::vector<glm::vec3> data(data_size_, {0, 0, 0});
	glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::vec3) * data_size_, data.data(), GL_STATIC_DRAW);
	generate_and_attach_texture(GL_RGB32F);
	initialized_ = true;
}

void texture_buffer_object::use(unsigned textureUnit) const
{
	if (!initialized_) return;
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_BUFFER, texture_);
}

bool texture_buffer_object::update_float(std::vector<float>* data, unsigned int offset)
{
	if (data_size_ < offset + data->size())
	{
		return false;
	}
	glBindBuffer(GL_TEXTURE_BUFFER, texture_buffer_);

	unsigned int size = sizeof(float) * data->size();
	void* ptr = glMapBufferRange(GL_TEXTURE_BUFFER, offset, size,
	                             GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	memcpy(ptr, data->data(), size);
	glUnmapBuffer(GL_TEXTURE_BUFFER);

	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	return true;
}

bool texture_buffer_object::update_vec3(std::vector<glm::vec3>* data, unsigned int offset)
{
	if (data_size_ < offset + data->size())
	{
		return false;
	}
	glBindBuffer(GL_TEXTURE_BUFFER, texture_buffer_);

	unsigned int size = sizeof(glm::vec3) * data->size();
	void* ptr = glMapBufferRange(GL_TEXTURE_BUFFER, offset, size,
	                             GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	memcpy(ptr, data->data(), size);
	glUnmapBuffer(GL_TEXTURE_BUFFER);

	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	
	return true;
}
