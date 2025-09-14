#pragma once
#include "glm.hpp"
#include <GL/glew.h>

template <typename T>
class uniform_buffer_object
{
public:
	void update_uniform_buffer(const T* data) const
	{
		if (!initialized_)
		{
			std::printf("UBO not initialized\n");
			return;
		}
		
		glBindBuffer(GL_UNIFORM_BUFFER, ubo_);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	
	void allocate_ubo(const GLenum usage, unsigned int binding)
	{
		if (initialized_)
		{
			std::printf("UBO already initialized\n");
			return;
		}
		glGenBuffers(1, &ubo_);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo_);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, usage);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo_);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		initialized_ = true;
	}

private:
	bool initialized_ = false;
	unsigned int ubo_ = 0;
};
