#include "AbstractVoxelizer.h"

AbstractVoxelizer::AbstractVoxelizer(Object3D* parent): Object3D(parent)
{
}

StructBoundingBox* AbstractVoxelizer::get_as_bounding_box()
{
	auto transform_global = getGlobalTransform();
	glm::vec3 upper_right_corner = round(transform_global * glm::vec4(1, 1, 1, 1));
	glm::vec3 lower_left_corner = round(transform_global * glm::vec4(-1, -1, -1, 1));
	return new StructBoundingBox{lower_left_corner, upper_right_corner};
}
