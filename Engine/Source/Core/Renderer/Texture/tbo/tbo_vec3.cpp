#include "tbo_vec3.h"
#include <GL/glew.h>

tbo_vec3::tbo_vec3(unsigned int data_size): texture_buffer_object(data_size)
{
	if (initialized_) { return; }
	generate_and_bind_buffer();
	// Step 2: Allocate and fill buffer with data
	std::vector<glm::vec3> data(data_size_, {0, 0, 0});
	glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::vec3) * data_size_, data.data(), GL_DYNAMIC_DRAW);
	generate_and_attach_texture(GL_RGB32F);
	initialized_ = true;
}


bool tbo_vec3::update_vec3(std::vector<glm::vec3>* data, unsigned int offset) const
{
	if (data_size_ < offset + data->size())
	{
		return false;
	}
	glBindBuffer(GL_TEXTURE_BUFFER, texture_buffer_);

	unsigned int size = sizeof(glm::vec3) * data->size();
	void* ptr = glMapBufferRange(GL_TEXTURE_BUFFER, sizeof(glm::vec3) * offset, size,
								 GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
	memcpy(ptr, data->data(), size);
	glUnmapBuffer(GL_TEXTURE_BUFFER);

	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	return true;
}
bool tbo_vec3::update_vec3_single(const glm::vec3* data, unsigned int offset) const
{
	if (data_size_ < offset + 1)
	{
		return false;
	}
	glBindBuffer(GL_TEXTURE_BUFFER, texture_buffer_);

	unsigned int size = sizeof(glm::vec3);
	void* ptr = glMapBufferRange(GL_TEXTURE_BUFFER, sizeof(glm::vec3) * offset, size,
								 GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
	memcpy(ptr, data, size);
	glUnmapBuffer(GL_TEXTURE_BUFFER);

	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	return true;
}
