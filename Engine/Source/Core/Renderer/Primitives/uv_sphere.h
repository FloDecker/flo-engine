#pragma once
#include "primitive.h"
#include "vertex_array.h"

class uv_sphere : public primitive
{
public:
	uv_sphere(int n_slices, int n_stacks);
	int load() override;
	int draw() override;

private:
	vertex_array* sphere_vertex_array_;
	primitive_type get_primitive_type() override;
};
