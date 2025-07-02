#pragma once

#include "../texture_buffer_object.h"

class tbo_float : public texture_buffer_object
{
public:
	tbo_float(unsigned int data_size);
	bool update_float(std::vector<float>* data, unsigned int offset) const;
	bool update_float_single(const float* data, unsigned int offset) const;
};
