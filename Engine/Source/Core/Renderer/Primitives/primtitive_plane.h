#pragma once

#include "primitive.h"
#include <GL/glew.h>
#include "../../CommonDataStructures/StructVertexArray.h"

class primtitive_plane : public primitive
{
public:
	primtitive_plane();
	static bool loaded;
	static unsigned int VBO;
	static unsigned int VAO;

	int load() override;
	int draw() override;
	static int draw_static();
	static int load_static();
	primitive_type get_primitive_type() override;

private:
	static vertex points_[4];
};
