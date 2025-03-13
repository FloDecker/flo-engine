#pragma once
#include "../Object3D.h"

struct gaussian
{
	glm::vec3 mean;
	glm::vec3 normal;
	float radius;
};


class gaussianinzer : public Object3D
{
public:
	explicit gaussianinzer(Object3D* parent)
		: Object3D(parent)
	{
	}

	void draw_object_specific_ui() override;

	unsigned int samples_per_meter = 2;
	float surface_attachment_radius = 1.0f;
	void snap_samples_to_closest_surface();
protected:
	void on_transform_changed() override;

private:
	std::vector<gaussian> samples_;
	void clear_samples();
	void calculate_gaussian();
	bool draw_debug_tools_;
	int amount_ = 100;

};
