#include "Cube.h"

#include <iostream>

bool Cube::loaded = false;
unsigned int Cube::VBO = 0;
unsigned int Cube::VAO = 0;


Cube::Cube()
{
	Cube::load();
}

int Cube::load()
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

int Cube::draw()
{
	if (!loaded)
	{
		std::cerr << "Draw-call issued for vertex array that hasn't been loaded yet" << '\n';
		return -1;
	}
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 24);
	glBindVertexArray(0);
	return 1;
}
