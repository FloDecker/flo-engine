#include "Remesh.h"


Remesh::remeshed_triangle Remesh::remesh_triangle(glm::vec3 triangles, unsigned int indices[3], unsigned int free_index)
{
	float distances[3] = {};
	for (int i = 0; i < 3; i++)
	{
		distances[i] = glm::distance(triangles[i], distances[(i + 1) % 3]);
	}

	int v_0 = 2;
	int v_1 = 0;

	if (distances[0] <= distances[1] && distances[0] <= distances[2])
	{
		v_0 = 0;
		v_1 = 1;
	}
	else if (distances[1] <= distances[0] && distances[1] <= distances[2])
	{
		v_0 = 1;
		v_1 = 2;
	}


	glm::vec3 new_center = glm::vec3(triangles[v_0] + triangles[v_1]) * 0.5f;

	if (v_0 == 0)
	{
		return {
			new_center,
			{indices[0], free_index, indices[2], free_index, indices[1], indices[2]},
			std::min(distances[0] * 0.5f, std::min(distances[1], distances[2]))
		};
	}

	if (v_0 == 1)
	{
		return {
			new_center,
			{indices[0], indices[1], free_index, indices[0], free_index, indices[2]},
			std::min(distances[1] * 0.5f, std::min(distances[0], distances[2]))
		};
	}

	return {
		new_center,
		{indices[0], indices[1], free_index, free_index, indices[1], indices[2]},
		std::min(distances[2] * 0.5f, std::min(distances[0], distances[1]))
	};
}

void Remesh::remesh_until_distance(vertex_array* vertex_array, float min_vertex_distance_ws)
{
}
