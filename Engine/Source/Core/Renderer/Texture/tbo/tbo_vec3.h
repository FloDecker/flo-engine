#pragma once
#include <vec3.hpp>

#include "../texture_buffer_object.h"

class tbo_vec3 :public texture_buffer_object
{
public:
	tbo_vec3(unsigned int data_size);
	bool update_vec3(std::vector<glm::vec3>* data, unsigned int offset) const;
	bool update_vec3_single(const glm::vec3* data, unsigned int offset) const;
};
