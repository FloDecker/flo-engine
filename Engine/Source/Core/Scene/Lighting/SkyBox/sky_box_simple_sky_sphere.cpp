#include "sky_box_simple_sky_sphere.h"


void sky_box_simple_sky_sphere::recalculate_colors()
{
	sky_box_ao_color_range_->colors.at(0) = color_ground;
	sky_box_ao_color_range_->colors.at(1) = color_horizon;
	sky_box_ao_color_range_->colors.at(2) = color_zenith;

	sky_box_ao_color_range_->sample_points.at(0) = 0.0;
	sky_box_ao_color_range_->sample_points.at(1) = 0.5;
	sky_box_ao_color_range_->sample_points.at(2) = 1.0;
}

void sky_box_simple_sky_sphere::draw_object_specific_ui()
{
	float c_0[3] = { color_ground.x,color_ground.y,color_ground.z };
	float c_1[3] = { color_horizon.x,color_horizon.y,color_horizon.z };
	float c_2[3] = { color_zenith.x,color_zenith.y,color_zenith.z };
	ImGui::ColorPicker3("Bottom color",c_0);
	ImGui::ColorPicker3("Horizon color",c_1);
	ImGui::ColorPicker3("Zenith color",c_2);
	color_ground = glm::vec3(c_0[0],c_0[1],c_0[2]);
	color_horizon = glm::vec3(c_1[0],c_1[1],c_1[2]);
	color_zenith = glm::vec3(c_2[0],c_2[1],c_2[2]);
	recalculate_colors();
}

int sky_box_simple_sky_sphere::drawSelf()
{
	sky_sphere_shader_->use();
	sky_sphere_shader_->add_header_uniforms(this, this->renderContext);
	glCullFace(GL_BACK);
	sky_sphere_->draw();
	glCullFace(GL_FRONT);
	return 1;
}
