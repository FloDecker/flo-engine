#include "uniform_buffer_object.h"

#include <gtc/type_ptr.hpp>


uniform_buffer_object::uniform_buffer_object()
{
	init();
}

void uniform_buffer_object::init()
{
	if (initialized_) return;
	
	//initialize UBO for direct light
	
	direct_light_ubo_location = allocate_ubo(sizeof(struct ubo_direct_light), GL_DYNAMIC_DRAW, 1);
	initialized_ = true;
	
}

void uniform_buffer_object::update_direct_light()
{
	update_uniform_buffer(direct_light_ubo_location, sizeof(struct ubo_direct_light), ubo_direct_light);
}

unsigned int uniform_buffer_object::allocate_ubo(const unsigned int size_byte, const GLenum usage, unsigned int binding)
{
	unsigned int ubo_temp;
	glGenBuffers(1, &ubo_temp);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_temp);
	glBufferData(GL_UNIFORM_BUFFER, size_byte, nullptr, usage);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo_temp);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return ubo_temp;
}

void uniform_buffer_object::update_uniform_buffer(const unsigned int ubo, const unsigned int size_byte, const void* data)
{
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, size_byte, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);  
}

