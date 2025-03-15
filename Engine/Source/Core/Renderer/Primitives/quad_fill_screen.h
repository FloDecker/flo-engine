#pragma once
#include "primitive.h"
#include "../../CommonDataStructures/StructVertexArray.h"

class quad_fill_screen : public primitive
{
public:
	int load() override;
	int draw() override;
	static bool loaded;
	static unsigned int VBO;
	static unsigned int VAO;

private:
	float quadVertices[24] = {
		// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};
};
