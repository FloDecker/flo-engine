#include "primtitive_plane.h"


bool primtitive_plane::loaded = false;
unsigned int primtitive_plane::VBO = 0;
unsigned int primtitive_plane::VAO = 0;

// A --- B
// |     |
// D --- C

vertex primtitive_plane::points_[4] = {

	{
		.position = glm::vec3(-0.5, 0.5, 0),
		.normal = glm::vec3(0, 0, 1),
		.tex_coords = glm::vec2(1, 0),
	},

	{
		.position = glm::vec3(-0.5, -0.5, 0),
		.normal = glm::vec3(0, 0, 1),
		.tex_coords = glm::vec2(0, 0),
	},
	{
		.position = glm::vec3(0.5, 0.5, 0),
		.normal = glm::vec3(0, 0, 1),
		.tex_coords = glm::vec2(1, 1),
	},

	{
		.position = glm::vec3(0.5, -0.5, 0),
		.normal = glm::vec3(0, 0, 1),
		.tex_coords = glm::vec2(0, 1),
	},


};


primtitive_plane::primtitive_plane()
{
	load_static();
}

int primtitive_plane::load()
{
	return load_static();
}

int primtitive_plane::draw()
{
	return draw_static();
}

int primtitive_plane::draw_static()
{
	if (!loaded)
	{
		load_static();
	}


	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	return 1;
}

int primtitive_plane::load_static()
{
	if (loaded)
	{
		return 0;
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);


	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(points_), points_, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), static_cast<void*>(nullptr)); //position - vec 3
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)sizeof(glm::vec3)); //normal   - vec 3
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(sizeof(glm::vec3) * 2)); //uv       - vec 2
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);


	loaded = true;
	return 1;
}

primitive_type primtitive_plane::get_primitive_type()
{
	return plane;
}
