#include "sky_box.h"

glm::vec3 sky_box::get_ambient_color_at(glm::vec3 outgoing_vector)
{
	return glm::vec3(0, 0, 0);
}

void sky_box::recalculate_colors()
{
}
