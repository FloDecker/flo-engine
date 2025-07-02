#pragma once
#include "texture.h"

class texture_buffer_object : public texture
{
public:
	//init for each datatype
	texture_buffer_object(unsigned int data_size);
	void use(unsigned textureUnit) const override;

	bool move_data(unsigned int from, unsigned int to, unsigned int length);

protected:
	unsigned int data_size_ = 0;
	unsigned int texture_buffer_ = -1;

	void generate_and_bind_buffer();
	void generate_and_attach_texture(int internal_format);
};
