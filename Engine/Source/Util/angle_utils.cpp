#include "angle_utils.h"

#include <corecrt_startup.h>
#include <assimp/MathFunctions.h>
#include <ext/quaternion_trigonometric.hpp>


float angle_utils::normalizeAngle(float angle) {
	constexpr float twoPi = 2.0 * glm::pi<float>();
	angle = fmod(angle, twoPi); // Use modulo to bring angle within [-2π, 2π)
	if (angle < 0) {
		angle += twoPi; // Adjust to [0, 2π)
	}
	return angle;
}

glm::vec3 angle_utils::normalize_angles_vec3(const glm::vec3 angles)
{
	return {glm::clamp(normalizeAngle(angles.x), -glm::pi<float>()*0.5f, glm::pi<float>()*0.5f),normalizeAngle(angles.y),normalizeAngle(angles.z)};
}

glm::quat angle_utils::vector_rotation_to_quat(const glm::vec3 rotation, const float angle_rad)
{
	return glm::angleAxis(angle_rad, rotation);
	const auto normalized_axis = normalize(rotation);
	const float half_ange = angle_rad / 2.0f;
	const float sin_half_angle = std::sin(half_ange);
	
	auto q = glm::quat();
	q.w = cos(half_ange);
	q.x = sin_half_angle * normalized_axis.x;
	q.y = sin_half_angle * normalized_axis.y;
	q.z = sin_half_angle * normalized_axis.z;
	
	return q;
}

glm::vec3 angle_utils::to_euler(glm::quat q) {
	glm::vec3 angles;

	// roll (x-axis rotation)
	double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	angles.x = std::atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	double sinp = std::sqrt(1 + 2 * (q.w * q.y - q.x * q.z));
	double cosp = std::sqrt(1 - 2 * (q.w * q.y - q.x * q.z));
	angles.y = 2 * std::atan2(sinp, cosp) - glm::pi<double>() / 2;

	// yaw (z-axis rotation)
	double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	angles.z = std::atan2(siny_cosp, cosy_cosp);

	return angles;
}