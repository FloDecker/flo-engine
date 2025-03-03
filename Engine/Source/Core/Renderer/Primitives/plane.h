#pragma once

#include "primitive.h"
#include <GL/glew.h>
#include "../../CommonDataStructures/StructVertexArray.h"

class plane : public primitive
{
public:
	static bool loaded;
	static unsigned int VBO;
	static unsigned int VAO;

	int load() override;
	int draw() override;
	static int draw_static();
	static int load_static();

private:
	static vertex points_[4];
};
