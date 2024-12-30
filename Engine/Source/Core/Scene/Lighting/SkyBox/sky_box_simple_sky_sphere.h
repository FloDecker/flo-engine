#pragma once
#include "sky_box.h"
#include "../../../Renderer/Primitives/uv_sphere.h"

static const std::string material_path_sky_sphere_ = "EngineContent/Shader/SkySphere.glsl";

class sky_box_simple_sky_sphere : public sky_box
{
public:
	glm::vec3 color_ground = glm::vec3(0.2,0.3,0.4);
	glm::vec3 color_horizon = glm::vec3(0.5294,0.8078,0.9215);
	glm::vec3 color_zenith = glm::vec3(0.3882,0.8980,0.9490);

	float color_ground_range = 0.0;
	float color_horizon_range = 0.5;
	float color_zenith_range = 1.0;

	
	explicit sky_box_simple_sky_sphere(Object3D* parent)
		: sky_box(parent)
	{
		setScale(512);
		sky_sphere_ = new uv_sphere(32, 16);
		sky_sphere_shader_ = new ShaderProgram();
		sky_sphere_shader_->loadFromFile(material_path_sky_sphere_);
		sky_sphere_shader_->set_shader_header_include(DYNAMIC_AMBIENT_LIGHT,true);
		sky_sphere_shader_->compileShader();
		name = "simple_sky_box";
		recalculate_colors();
	}

	void recalculate_colors() override;

	void draw_object_specific_ui() override;

protected:
	uv_sphere* sky_sphere_;
	ShaderProgram* sky_sphere_shader_;
	int drawSelf() override;
};
