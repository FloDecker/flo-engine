//
// Created by flode on 28/02/2023.
//
#include "vertex_array.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/string_cast.hpp"


vertex_array::vertex_array(std::vector<vertex>* vertices, std::vector<unsigned int>* indices)
{
	this->vertex_array_.vertices = vertices;
	this->vertex_array_.indices = indices;
}


int vertex_array::load()
{
	if (loaded)
	{
		return 0;
	}
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertex_array_.vertices->size() * sizeof(vertex), vertex_array_.vertices->data(),
	             GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertex_array_.indices->size() * sizeof(int), vertex_array_.indices->data(),
	             GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), static_cast<void*>(nullptr)); //position - vec 3
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)sizeof(glm::vec3)); //normal   - vec 3
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(sizeof(glm::vec3) * 2)); //uv       - vec 2
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	loaded = true;
	return 1;
}

//draws object with currently used shader
int vertex_array::draw()
{
	if (!loaded)
	{
		std::cerr << "Draw-call issued for vertex array that hasn't been loaded yet" << '\n';
		return -1;
	}

	glBindVertexArray(VAO);
	glPolygonMode(GL_FRONT_AND_BACK, mode_);
	glDrawElements(GL_TRIANGLES, vertex_array_.indices->size(), GL_UNSIGNED_INT, nullptr);

	glBindVertexArray(0);
	return 1;
}

bool vertex_array::set_draw_mode(GLenum mode)
{
	if (mode != GL_POINT && mode != GL_LINE && mode != GL_FILL)
	{
		std::cout << "Draw mode has to be of type: GL_POINT, GL_LINE or GL_FILL\n";
		return false;
	}
	mode_ = mode;
	return true;
}

struct_vertex_array* vertex_array::get_vertex_array()
{
	return &vertex_array_;
}

primitive_type vertex_array::get_primitive_type()
{
	return mesh;
}
