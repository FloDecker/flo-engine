#pragma once

#include <vec3.hpp>
#include <vector>

#include "texture.h"

class texture_buffer_object : public texture
{
public:
	bool init(int data_size, std::vector<float>* data);
	bool init(int data_size, std::vector<glm::vec3>* data);
	void use(unsigned textureUnit) const override;

private:
	unsigned int data_size_;


	void generate_and_bind_buffer();
	void generate_and_attach_texture(int internal_format);

	void init_float_(std::vector<float>* data);
	void init_vec3_(std::vector<glm::vec3>* data);

	unsigned int texture_buffer_;
};
