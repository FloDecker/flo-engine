#pragma once
#include <vec3.hpp>

class Object3D;
class Scene;

class visual_debug_tools
{
public:
	explicit visual_debug_tools(Scene* scene);
	void draw_debug_line(glm::vec3 pos_start, glm::vec3 pos_end, glm::vec3 color, float time = 0) const;
	void draw_debug_cube(glm::vec3 position, glm::vec3 size, glm::vec3 color, glm::vec3 rotation, float time = 0);

private:
	Scene* scene_;
	Object3D* debug_objects_root_;
};
