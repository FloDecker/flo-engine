#pragma once
#include "../Object3D.h"

class light : public Object3D
{
public:
	light(Object3D* parent);

	glm::mat4 get_light_space_matrix();
	float intensity;
	glm::vec3 color;
	virtual void render_to_light_map(); //called by the scene to set the render targets for the corresponding light pass
protected:
	virtual void on_light_changed();
	void on_transform_changed() override;
	glm::mat4 light_matrix_;
};
