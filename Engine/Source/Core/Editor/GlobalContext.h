#pragma once
#include <string>
#include <vector>

#include "../Renderer/Primitives/Cube.h"
#include "../Renderer/Primitives/Line.h"
#include "../Renderer/Shader/ShaderProgram.h"


class TagManager
{
public:
	TagManager();
	int add_tag(std::string);
	int get_id_of_tag(const std::string& tag) const;

private:
	std::vector<std::string> tags_;
	const std::vector<const char*> ENGINE_TAGS{"ENGINE_LIGHT_POINT", "ENGINE_COLLIDER", "ENGINE_HANDLE_COLLIDER"};
};

//primitives used for debugging
struct debug_primitives
{
	Cube* cube;
	Line* line;
};

//Global context holds information that is needed across levels, such as tags 
struct GlobalContext
{
	TagManager tag_manager;
	debug_primitives debug_primitives;

	ShaderProgram* default_shader;
	ShaderProgram* default_color_debug_shader;
	ShaderProgram* light_pass_depth_only_shader;
};
