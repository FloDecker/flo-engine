#pragma once
#include "glm.hpp"
#include <vec3.hpp>
#include <GL/glew.h>

struct ubo_direct_light
{
	alignas(16) glm::vec3 light_direction;
	alignas(4) float light_intensity;
	alignas(16)glm::vec3 light_color;
	alignas(4) float light_angle;
	alignas(16) glm::mat4 direct_light_light_space_matrix;

};

class uniform_buffer_object
{
public:
	uniform_buffer_object();
	void init();
	void update_direct_light();
	ubo_direct_light* ubo_direct_light = new struct ubo_direct_light;

private:
	bool initialized_ = false;
	unsigned int allocate_ubo(unsigned int size_byte, GLenum usage, unsigned int binding);
	void update_uniform_buffer(unsigned int ubo, unsigned int size_byte, const void* data);

	//current UBO's

	//direcet light
	unsigned int direct_light_ubo_location;
};
