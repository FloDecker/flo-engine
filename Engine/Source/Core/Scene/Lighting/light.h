#pragma once
#include "../Object3D.h"

class light : public Object3D
{
public:
	light(Object3D* parent);

	glm::mat4 get_light_space_matrix();
	float intensity = 1.0f;
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	virtual void render_to_light_map(); //called by the scene to set the render targets for the corresponding light pass
	void draw_object_specific_ui() override;

protected:
	virtual void on_light_changed();
	void on_transform_changed() override;
	glm::mat4 light_matrix_;
};
