#include "uniform_buffer_object.h"
#include <GL/glew.h>

void uniform_buffer_object::init()
{
	
}
/**
void uniform_buffer_object::allocate_ubo(unsigned int *ubo, const unsigned int size_byte, const GLenum usage)
{
	glGenBuffers(1, ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, *ubo);
	glBufferData(GL_UNIFORM_BUFFER, size_byte, nullptr, usage);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
*/