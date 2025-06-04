#pragma once
#include <vec3.hpp>


struct surfel_octree_metadata;

struct surfel
{
	glm::vec3 mean;
	glm::vec3 normal;
	glm::vec3 diffuse_irradiance;
	float radius;

	//vector of pointers to buckets that are containing this surfel
	std::vector<unsigned int> in_surfel_buckets = std::vector<unsigned int>();

	//how often this surfel was sampled to obtain the current color 
	int samples = 0;
};
