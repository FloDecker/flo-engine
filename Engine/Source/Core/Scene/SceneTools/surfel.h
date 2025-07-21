#pragma once
#include <vec3.hpp>
#include <vec2.hpp>


struct surfel_octree_metadata;

constexpr glm::vec2 surfel_sample_offsets[4] = {
	{1,0},
	{-1,0},
	{0,1},
	{0,-1}
};

struct surfel
{
	glm::vec3 mean;
	glm::vec3 normal;
	glm::vec3 diffuse_irradiance;
	float radius;

	//vector of pointers to buckets that contain this surfel
	std::vector<unsigned int> in_surfel_buckets = std::vector<unsigned int>();

	//how often this surfel was sampled to obtain the current radiance_ambient 
	int samples = 0;

	
	glm::vec3 diffuse_irradiance_samples[4] =  {};

	
};
