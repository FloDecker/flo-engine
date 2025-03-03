#include "plane.h"


bool plane::loaded = false;
unsigned int plane::VBO = 0;
unsigned int plane::VAO = 0;

// A --- B
// |     |
// D --- C

vertex plane::points_[4] = {

	{
		.position= glm::vec3(-0.5, 0.5, 0),
		.normal= glm::vec3(0, 0, 1),
		.tex_coords= glm::vec2(1, 0),
	},
	{
		.position= glm::vec3(0.5, 0.5, 0),
		.normal= glm::vec3(0, 0, 1),
		.tex_coords= glm::vec2(1, 1),
	},

	{
		.position= glm::vec3(0.5, -0.5, 0),
		.normal= glm::vec3(0, 0, 1),
		.tex_coords= glm::vec2(0, 1),
	},
	{
		.position= glm::vec3(-0.5, -0.5, 0.5),
		.normal= glm::vec3(0, 0, 1),
		.tex_coords= glm::vec2(0, 0),
	},
};


int plane::load()
{
	return load_static();
}

int plane::draw()
{
	return draw_static();
}

int plane::draw_static()
{
	if (!loaded)
	{
		load_static();
	}


	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 4);
	glBindVertexArray(0);
	return 1;
}

int plane::load_static()
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
