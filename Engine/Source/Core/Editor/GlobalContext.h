#pragma once
#include <string>
#include <vector>

#include "performance_metrics.h"
#include "../Renderer/uniform_buffer_object.h"
#include "../Renderer/Primitives/Cube.h"
#include "../Renderer/Primitives/Line.h"
#include "../Renderer/Primitives/primtitive_plane.h"
#include "../Renderer/Shader/ShaderProgram.h"

enum log_type
{
	log_info,
	log_warning,
	log_error
};

inline const char* to_string(log_type e)
{
	switch (e)
	{
	case log_info: return "log_info";
	case log_warning: return "log_warning";
	case log_error: return "log_error";
	default: return "unknown";
	}
}

struct log_entry
{
	log_type type;
	std::string log;
};

class Logger
{
public:
	unsigned int max_logs = 1000;
	unsigned int get_current_log_amount() const;
	std::vector<log_entry>* get_log_entries();
	void print_info(std::string log);
	void print_warning(std::string log);
	void print_error(std::string log);

private:
	std::vector<log_entry> log_array_;
	void print_to_log(log_type type, std::string log);
};

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
struct global_primitives
{
	Cube* cube;
	Line* line;
	primtitive_plane* plane;
};

//Global context holds information that is needed across levels, such as tags 
struct GlobalContext
{
	TagManager tag_manager;
	Logger* logger = new Logger();
	performance_metrics* performance_metrics = new ::performance_metrics();
	global_primitives global_primitives;
	uniform_buffer_object* uniform_buffer_object;

	//global shaders
	ShaderProgram* default_shader;
	ShaderProgram* default_color_debug_shader;
	ShaderProgram* light_pass_depth_only_shader;
};
