#include "sky_box_atmospheric_scattering.h"

int sky_box_atmospheric_scattering::drawSelf()
{
	sky_sphere_shader_->use();
	sky_sphere_shader_->add_header_uniforms(this, this->renderContext);
	glCullFace(GL_BACK);
	sky_sphere_->draw();
	glCullFace(GL_FRONT);
	return 1;
}
