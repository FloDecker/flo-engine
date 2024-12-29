#include "sky_box_simple_sky_sphere.h"

glm::vec3 sky_box_simple_sky_sphere::get_ambient_color_at(glm::vec3 outgoing_vector)
{
	return sky_box::get_ambient_color_at(outgoing_vector);
}

void sky_box_simple_sky_sphere::recalculate_colors()
{
	sky_box::recalculate_colors();
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
