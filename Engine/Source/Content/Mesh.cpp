#include "Mesh.h"

void Mesh::initializeVertexArrays() const
{
	for (auto element : vertexArrays)
	{
		element->load();
	}
}
