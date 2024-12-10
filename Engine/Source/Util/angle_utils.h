#pragma once
#include <fwd.hpp>
#include <detail/type_quat.hpp>

class angle_utils
{
public:
	static float normalizeAngle(float angle);
	static glm::vec3 normalize_angles_vec3(glm::vec3 angles);
	static glm::quat vector_rotation_to_quat(glm::vec3 rotation, float angle_rad);
	static glm::vec3 to_euler(glm::quat q);

};
