#pragma once

#include <vec3.hpp>
#include <vector>

#include "texture.h"

class texture_buffer_object : public texture
{
public:
	//init for each datatype 
	bool init_float(unsigned int data_size);
	bool init_vec3(unsigned int data_size);
	
	void use(unsigned textureUnit) const override;

	//update for each datatype
	bool update_float(std::vector<float>* data, unsigned int offset);
	bool update_vec3(std::vector<glm::vec3>* data, unsigned int offset);

private:
	unsigned int data_size_ = 0;
	
	unsigned int texture_buffer_ = -1;

	void generate_and_bind_buffer();
	void generate_and_attach_texture(int internal_format);
	
};
