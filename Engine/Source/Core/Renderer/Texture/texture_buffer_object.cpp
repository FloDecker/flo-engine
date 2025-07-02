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

texture_buffer_object::texture_buffer_object(unsigned int data_size)
{
	this->data_size_ = data_size;
}


void texture_buffer_object::use(unsigned textureUnit) const
{
	if (!initialized_) return;
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_BUFFER, texture_);
}


bool texture_buffer_object::move_data(unsigned int from, unsigned int to, unsigned int length)
{
	GLuint tempBuffer;
	glCreateBuffers(1, &tempBuffer);
	glNamedBufferData(tempBuffer, length, nullptr, GL_STATIC_COPY);

	// Copy to temp
	glCopyNamedBufferSubData(texture_buffer_, tempBuffer, from, 0, length);

	// Copy from temp to destination range
	glCopyNamedBufferSubData(tempBuffer, texture_buffer_, 0, to, length);

	glDeleteBuffers(1, &tempBuffer);

	return true;
}
