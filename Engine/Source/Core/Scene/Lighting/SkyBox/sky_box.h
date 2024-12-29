#pragma once
#include <vec3.hpp>

#include "../../Object3D.h"
#include "../../Scene.h"

class sky_box: public Object3D
{
public:
	explicit sky_box(Object3D* parent)
		: Object3D(parent)
	{
		parent->get_scene()->register_sky_box(this);
		sky_box_ao_color_range_ = new StructColorRange {
			.sample_amount = 3,
			.colors = std::vector<glm::vec3>(3),
			.sample_points = std::vector<float>(3)
		};
	}
	
	StructColorRange* get_sky_box_ao_color_range();
	
	//function called by scene when sky parameters change
	virtual void recalculate_colors();
protected:
	StructColorRange* sky_box_ao_color_range_;
};
