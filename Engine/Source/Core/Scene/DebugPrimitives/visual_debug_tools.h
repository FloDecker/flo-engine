#pragma once
#include <vec3.hpp>

class Line3D;
class Object3D;
class Scene;

struct debug_line_data
{
	glm::vec3 start;
	glm::vec3 end;
	glm::vec3 color;
	float ttl;
};

class visual_debug_tools
{
public:
	explicit visual_debug_tools(Scene* scene);
	void draw_debug_line(glm::vec3 pos_start, glm::vec3 pos_end, glm::vec3 color, float time = 0);
	void draw_debug_cube(glm::vec3 position, glm::vec3 size, glm::vec3 color, glm::vec3 rotation, float time = 0);
	void draw_debug_tools(float delta);

private:
	Scene* scene_;
	Object3D* debug_objects_root_;
	std::vector<debug_line_data> debug_lines_ ;
	Line3D *line_3d_;
};
