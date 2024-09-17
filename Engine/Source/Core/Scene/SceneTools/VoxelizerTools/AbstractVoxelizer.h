#pragma once
#include "../../Object3D.h"
#include "../../SceneContext.h"
class AbstractVoxelizer : public Object3D
{
public:
	explicit AbstractVoxelizer(GlobalContext* global_context, SceneContext* scene_context);
	
	int voxel_precision; //how many segments are put into a 1x1 units square
	virtual void recalculate() {std::wcout<<"recalculate not implemented\n";} //recalculate the voxel field
	virtual void load_into_voxel_texture(Texture3D* texture_3d) {std::wcout<<"load_into_voxel_texture not implemented\n";} //load voxel data into 3d texture

	StructBoundingBox* get_as_bounding_box();
	//for debug
	bool show_voxels = true; //TODO:not implemented
private:
protected:
	SceneContext* scene_context_;

};
