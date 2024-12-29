#pragma once
#include "sky_box.h"
#include "../../../Renderer/Primitives/uv_sphere.h"

static const std::string material_path_ = "EngineContent/Shader/SkySphere.glsl";

class sky_box_simple_sky_sphere : public sky_box
{
public:
	glm::vec3 color_ground = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 color_horizon = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 color_zenith = glm::vec3(0.0f, 0.0f, 0.0f);

	explicit sky_box_simple_sky_sphere(Object3D* parent)
		: sky_box(parent)
	{
		setScale(512);
		sky_sphere_ = new uv_sphere(32, 16);
		sky_sphere_shader_ = new ShaderProgram();
		sky_sphere_shader_->loadFromFile(material_path_);
		sky_sphere_shader_->compileShader();
	}

	glm::vec3 get_ambient_color_at(glm::vec3 outgoing_vector) override;
	void recalculate_colors() override;

protected:
	uv_sphere* sky_sphere_;
	ShaderProgram* sky_sphere_shader_;
	int drawSelf() override;
};
