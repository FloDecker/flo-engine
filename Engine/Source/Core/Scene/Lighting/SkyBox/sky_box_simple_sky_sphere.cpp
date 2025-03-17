#include "sky_box_simple_sky_sphere.h"


void sky_box_simple_sky_sphere::recalculate_colors()
{
	sky_box_ao_color_range_->colors.at(0) = color_ground;
	sky_box_ao_color_range_->colors.at(1) = color_horizon;
	sky_box_ao_color_range_->colors.at(2) = color_zenith;

	sky_box_ao_color_range_->sample_points.at(0) = color_ground_range;
	sky_box_ao_color_range_->sample_points.at(1) = color_horizon_range;
	sky_box_ao_color_range_->sample_points.at(2) = color_zenith_range;
}

void sky_box_simple_sky_sphere::draw_object_specific_ui()
{
	bool changed = false;
	changed = changed || ImGui::ColorPicker3("Bottom color", &color_ground[0]);
	changed = changed || ImGui::SliderFloat("Bottom color", &color_ground_range, 0, 1);
	changed = changed || ImGui::ColorPicker3("Horizon color", &color_horizon[0]);
	changed = changed || ImGui::SliderFloat("Horizon color", &color_horizon_range, 0, 1);
	changed = changed || ImGui::ColorPicker3("Zenith color", &color_zenith[0]);
	changed = changed || ImGui::SliderFloat("Zenith color", &color_zenith_range, 0, 1);

	if (changed) recalculate_colors();
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
