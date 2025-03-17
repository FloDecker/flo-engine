#pragma once
#include "sky_box.h"
#include "../../../Renderer/Primitives/uv_sphere.h"

static const std::string material_path_ = "EngineContent/Shader/SkySphereAtmosphericScattering.glsl";

class sky_box_atmospheric_scattering : public sky_box
{
public:
	explicit sky_box_atmospheric_scattering(Object3D* parent)
		: sky_box(parent)
	{
		setScale(512);
		sky_sphere_ = new uv_sphere(32, 16);
		sky_sphere_shader_ = new ShaderProgram();
		sky_sphere_shader_->loadFromFile(material_path_);
		sky_sphere_shader_->set_shader_header_include(DYNAMIC_DIRECTIONAL_LIGHT, true);
		sky_sphere_shader_->set_shader_header_include(DYNAMIC_AMBIENT_LIGHT, true);
		sky_sphere_shader_->compileShader();
		name = "atmospheric_scattering_sky_box";
		recalculate_colors();
	}

protected:
	uv_sphere* sky_sphere_;
	ShaderProgram* sky_sphere_shader_;
	int drawSelf() override;
};
