#include "tbo_float.h"
#include <GL/glew.h>

tbo_float::tbo_float(unsigned int data_size): texture_buffer_object(data_size)
{
	if (initialized_) { return; }
	generate_and_bind_buffer();
	// Step 2: Allocate and fill buffer with data
	std::vector<float> data(data_size_, 0);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(float) * data_size_, data.data(), GL_DYNAMIC_DRAW);
	generate_and_attach_texture(GL_R32F);
	initialized_ = true;
}


bool tbo_float::update_float(std::vector<float>* data, unsigned int offset) const
{
	if (data_size_ < offset + data->size())
	{
		return false;
	}
	glBindBuffer(GL_TEXTURE_BUFFER, texture_buffer_);

	unsigned int size = sizeof(float) * data->size();
	void* ptr = glMapBufferRange(GL_TEXTURE_BUFFER, sizeof(float) * offset, size,
	                             GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
	memcpy(ptr, data->data(), size);
	glUnmapBuffer(GL_TEXTURE_BUFFER);

	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	return true;
}

bool tbo_float::update_float_single(const float* data, unsigned int offset) const
{
	if (data_size_ < offset + 1)
	{
		return false;
	}
	glBindBuffer(GL_TEXTURE_BUFFER, texture_buffer_);

	unsigned int size = sizeof(float);
	void* ptr = glMapBufferRange(GL_TEXTURE_BUFFER, size * offset, size,
	                             GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
	memcpy(ptr, data, size);
	glUnmapBuffer(GL_TEXTURE_BUFFER);

	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	return true;
}
