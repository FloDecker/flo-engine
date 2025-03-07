#include "visual_debug_tools.h"

#include "Line3D.h"
#include "Cube3D.h"

#include "../Scene.h"
#include "../Object3D.h"

visual_debug_tools::visual_debug_tools(Scene* scene) : scene_(scene)
{
	debug_objects_root_ = new Object3D(scene_->get_root());
	debug_objects_root_->name = ".EngineInternal_DebugRoot";

	//setup line
	line_3d_ = new Line3D(debug_objects_root_, glm::vec3(),
	                      glm::vec3());
	line_3d_->visible = false;

	//setup cube
	cube_3d_ = new Cube3D(debug_objects_root_);
	cube_3d_->visible = false;
}

void visual_debug_tools::draw_debug_line(glm::vec3 pos_start, glm::vec3 pos_end, glm::vec3 color = {1, 0, 0},
                                         float time)
{
	debug_lines_.push_back({pos_start, pos_end, color, time});
}

void visual_debug_tools::draw_debug_point(glm::vec3 position, float time, glm::vec3 color, float size)
{
	debug_cubes_.push_back({position, glm::quat(), {size, size, size}, color, time});
}

void visual_debug_tools::draw_debug_cube(glm::vec3 position, float time, glm::quat rot, glm::vec3 scale,
                                         glm::vec3 color)
{
	debug_cubes_.push_back({position, rot, scale, color, time});
}

void visual_debug_tools::draw_debug_tools(float delta)
{
	//draw lines 
	line_3d_->visible = true;
	for (auto& debug_line : debug_lines_)
	{
		const auto line = debug_line;
		line_3d_->set_positions(line.start, line.end);
		line_3d_->color = line.color;
		line_3d_->drawSelf();
		debug_line.ttl -= delta;
	}

	//draw cubes 
	cube_3d_->visible = true;
	for (auto& debug_cube : debug_cubes_)
	{
		const auto cube = debug_cube;
		cube_3d_->setPositionLocal(cube.pos);
		cube_3d_->setRotationLocal(cube.rot);
		cube_3d_->setScale(cube.scale);
		cube_3d_->color = cube.color;
		cube_3d_->drawSelf();
		debug_cube.ttl -= delta;
	}


	std::erase_if(debug_lines_, [](const debug_line_data& l) { return l.ttl < 0.0f; });
	std::erase_if(debug_cubes_, [](const debug_cube_data& l) { return l.ttl < 0.0f; });


	line_3d_->visible = false;
	cube_3d_->visible = false;
}
