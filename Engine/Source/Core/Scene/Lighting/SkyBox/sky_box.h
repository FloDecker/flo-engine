#pragma once
#include <vec3.hpp>

#include "../../Object3D.h"

class sky_box: public Object3D
{
public:
	explicit sky_box(Object3D* parent)
		: Object3D(parent)
	{
	}

	//return ambient sky color for given outgoing vector
	virtual glm::vec3 get_ambient_color_at(glm::vec3 outgoing_vector);

	//function called by scene when sky parameters change
	virtual void recalculate_colors();
	
};
