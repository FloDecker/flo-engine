#include "visual_debug_tools.h"

#include "Line3D.h"
#include "../Scene.h"
#include "../Object3D.h"

visual_debug_tools::visual_debug_tools(Scene* scene) : scene_(scene)
{
	debug_objects_root_ = new Object3D(scene_->get_root());
	debug_objects_root_->name = ".EngineInternal_DebugRoot";
	line_3d_ = new Line3D(debug_objects_root_, glm::vec3(),
						   glm::vec3());
	line_3d_->visible = false;
}

void visual_debug_tools::draw_debug_line(glm::vec3 pos_start, glm::vec3 pos_end, glm::vec3 color = {1, 0, 0},
                                         float time)
{
	debug_lines_.push_back({pos_start,pos_end,color,time});
	//auto line = new Line3D(debug_objects_root_, scene_->get_global_context()->debug_primitives.line, pos_start,
	//                       pos_end);
	//line->color = color;
}

void visual_debug_tools::draw_debug_tools(float delta)
{
	line_3d_->visible = true;
	for (auto& debug_line : debug_lines_)
	{
		const auto line = debug_line;
		line_3d_->set_positions(line.start,line.end);
		line_3d_->color = line.color;
		line_3d_->drawSelf();
		debug_line.ttl-=delta;
	}

	std::erase_if(debug_lines_, [](const debug_line_data& l) { return l.ttl < 0.0f; });
	line_3d_->visible = false;
}
