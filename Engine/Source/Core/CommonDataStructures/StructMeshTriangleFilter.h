#pragma once
#include <vec3.hpp>
#include <vector>


struct vertex_array_filter {
	int vertex_array_id = -1;
	std::vector<unsigned int> indices = {};
};