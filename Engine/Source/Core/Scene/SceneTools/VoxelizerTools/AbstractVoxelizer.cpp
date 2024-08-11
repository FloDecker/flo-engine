#include "AbstractVoxelizer.h"

AbstractVoxelizer::AbstractVoxelizer(GlobalContext* global_context, SceneContext* scene_context): Object3D(global_context)
{
	scene_context_ = scene_context;
}
