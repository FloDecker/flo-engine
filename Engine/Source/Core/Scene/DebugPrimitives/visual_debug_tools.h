#pragma once
#include <detail/type_quat.hpp>
#include "glm.hpp"

struct StructBoundingBox;
class Line3D;
class Cube3D;
class Object3D;
class Scene;

struct debug_line_data
{
	glm::vec3 start;
	glm::vec3 end;
	glm::vec3 color;
	float ttl;
};

struct debug_cube_data
{
	glm::vec3 pos = {0,0,0};
	glm::quat rot = glm::quat();
	glm::vec3 scale = {1,1,1};
	
	glm::vec3 color = {1,1,1};
	
	float ttl;
};


class visual_debug_tools
{
public:
	explicit visual_debug_tools(Scene* scene);
	void draw_debug_line(glm::vec3 pos_start, glm::vec3 pos_end, glm::vec3 color, float time = 0);
	void draw_debug_point(glm::vec3 position, float time = 0, glm::vec3 color = {1,1,1}, float size = 0.05);
	void draw_debug_cube(glm::vec3 position, float time = 0, glm::quat rot = glm::quat(), glm::vec3 scale = {1,1,1}, glm::vec3 color = {1,1,1});
	void draw_debug_cube(StructBoundingBox *bb,float time = 0, glm::vec3 color = {1,1,1});
	void draw_debug_tools(float delta);

private:
	Scene* scene_;
	Object3D* debug_objects_root_;
	std::vector<debug_line_data> debug_lines_ ;
	std::vector<debug_cube_data> debug_cubes_ ;
	Line3D *line_3d_;
	Cube3D *cube_3d_;
};
