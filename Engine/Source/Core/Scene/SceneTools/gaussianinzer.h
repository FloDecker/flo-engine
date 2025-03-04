#pragma once
#include "../Object3D.h"

struct gaussian
{
	glm::vec3 mean;
	glm::vec3 normal;
	float radius;
};



struct sample
{
	glm::vec3 sample_pos;
	bool has_gaussian = false;
	gaussian *gaussian_at_sample = nullptr;
};


class gaussianinzer : public Object3D
{
public:
	explicit gaussianinzer(Object3D* parent)
		: Object3D(parent)
	{
	}

	unsigned int samples_per_meter = 2;
	void get_sample_positions_sparse();

protected:
	void on_transform_changed() override;

private:
	std::vector<sample> samples_;
	void clear_samples();
	void calculate_gaussian();

};
