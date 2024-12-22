#include "visual_debug_tools.h"

#include "Line3D.h"
#include "../Scene.h"
#include "../Object3D.h"

visual_debug_tools::visual_debug_tools(Scene* scene) : scene_(scene)
{
	debug_objects_root_ = new Object3D(scene_->get_root());
	debug_objects_root_->name = ".EngineInternal_DebugRoot";
}

void visual_debug_tools::draw_debug_line(glm::vec3 pos_start, glm::vec3 pos_end, glm::vec3 color = {1, 0, 0},
                                         float time) const
{
	auto line = new Line3D(debug_objects_root_, scene_->get_global_context()->debug_primitives.line, pos_start,
	                       pos_end);
	line->color = color;
}
