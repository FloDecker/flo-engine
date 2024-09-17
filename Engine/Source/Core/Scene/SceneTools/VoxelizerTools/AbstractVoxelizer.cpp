#include "AbstractVoxelizer.h"

AbstractVoxelizer::AbstractVoxelizer(GlobalContext* global_context, SceneContext* scene_context): Object3D(global_context)
{
	scene_context_ = scene_context;
}

StructBoundingBox* AbstractVoxelizer::get_as_bounding_box()
{
	auto transform_global = getGlobalTransform();
	glm::vec3 upper_right_corner = glm::round(transform_global * glm::vec4(1, 1, 1, 1));
	glm::vec3 lower_left_corner = glm::round(transform_global * glm::vec4(-1, -1, -1, 1));
	return new StructBoundingBox{lower_left_corner,upper_right_corner};
}
