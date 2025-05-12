#pragma once
#include "../Object3D.h"
#include "../Scene.h"
#include "gaussian.h"

class gaussianizer : public Object3D
{
public:
	explicit gaussianizer(Object3D* parent)
		: Object3D(parent)
	{
		parent->get_scene()->register_gaussianizer(this);
		name = "gaussianizer";
	}

	void draw_object_specific_ui() override;

	unsigned int samples_per_meter = 2;
	float surface_attachment_radius = 1.0f;
	void snap_samples_to_closest_surface();
	std::vector<gaussian> samples() const;

protected:
	void on_transform_changed() override;

private:
	std::vector<gaussian> samples_;
	void clear_samples();
	void calculate_gaussian();
	bool draw_debug_tools_;
	int points_per_square_meter = 1;
};
