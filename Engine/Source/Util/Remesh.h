#pragma once
#include <vec3.hpp>

#include "../Core/Renderer/VertexArray.h"

class Remesh
{
	struct remeshed_triangle
	{
		glm::vec3 new_vertex;
		unsigned int indices[6];
		float smallest_edge_distance;
	};
	
public:
	static remeshed_triangle remesh_triangle(glm::vec3 triangles, unsigned int indices[3], unsigned int free_index);
	static void remesh_until_distance(VertexArray* vertex_array, float min_vertex_distance_ws);
};
