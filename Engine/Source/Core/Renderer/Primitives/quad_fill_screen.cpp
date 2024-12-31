#include "quad_fill_screen.h"

#include <iostream>
#include <GL/glew.h>


bool quad_fill_screen::loaded = false;
unsigned int quad_fill_screen::VBO = 0;
unsigned int quad_fill_screen::VAO = 0;
int quad_fill_screen::load()
{
	if (loaded)
	{
		return 0;
	}
	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    
	loaded = true;
	return 1;
}


int quad_fill_screen::draw()
{
	if (!loaded) {
		std::cerr << "Draw-call issued for vertex array that hasn't been loaded yet" << '\n';
		return -1;
	}
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES,0,6);
	glBindVertexArray(0);
	return 1;
}
