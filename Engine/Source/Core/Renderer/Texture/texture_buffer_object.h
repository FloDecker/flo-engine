#pragma once

#include <glm.hpp>
#include <vec3.hpp>
#include <vector>
#include "texture.h"

class texture_buffer_object : public texture
{
public:
	//init for each datatype 
	bool init_float(unsigned int data_size);
	bool init_vec3(unsigned int data_size);
	bool init_u_int_2(unsigned int data_size);
	bool init_u_int(unsigned int data_size);

	void use(unsigned textureUnit) const override;

	//update for each datatype
	bool update_float(std::vector<float>* data, unsigned int offset);
	bool update_float_single(const float* data, unsigned int offset);
	bool update_vec3(std::vector<glm::vec3>* data, unsigned int offset);
	bool update_vec3_single(const glm::vec3* data, unsigned int offset);
	bool update_u_int_2(std::vector<glm::u32vec2>* data, unsigned int offset);
	bool update_u_int(std::vector<uint32_t>* data, unsigned int offset);
	bool update_u_int(uint32_t* data, unsigned int length, unsigned int offset);

	bool move_data(unsigned int from, unsigned int to, unsigned int length);

private:
	unsigned int data_size_ = 0;

	unsigned int texture_buffer_ = -1;

	void generate_and_bind_buffer();
	void generate_and_attach_texture(int internal_format);
};
