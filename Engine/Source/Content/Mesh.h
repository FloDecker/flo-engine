//
// Created by flode on 14/03/2023.
//

#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#include <vector>
#include "../Core/Renderer/Primitives/vertex_array.h"
#include "Material.h"

class Mesh
{
	glm::vec3 center_of_mass_ = {0, 0, 0};
	glm::mat3 inertia_tensor_ = glm::mat3(0);
	bool mesh_calculation_ran_ = false;

public:
	Mesh()
	{
		vertexArrays = {};
		materials = {};
	}

	std::vector<vertex_array*> vertexArrays;
	std::vector<ShaderProgram*> materials;
	void initializeVertexArrays() const;
};

#endif //ENGINE_MESH_H
