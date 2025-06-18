#pragma once
#include <format>

auto name_direct_light_texture_sampler = "direct_light_texture_sampler";
auto name_direct_light_direction = "direct_light_direction";
auto name_direct_light_intensity = "direct_light_intensity";
auto name_direct_light_color = "direct_light_color";


auto name_u_ambient_light_colors = "u_ambient_light_colors";
auto name_ambient_light_colors_sample_positions = "u_ambient_light_colors_sample_positions";


//header for standard vertex shader
auto VERTEX_SHADER_HEADER_BASE =
	"#version 430 core \n"
	"layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec3 aNormal;\n"
	"layout (location = 2) in vec2 aUV;\n"
	"uniform mat4 mMatrix;\n"
	"uniform mat4 vMatrix;\n"
	"uniform mat4 pMatrix;\n"
	"uniform vec3 cameraPosWS;\n";

//header for standard fragment shader
auto FRAGMENT_SHADER_HEADER_BASE =
	"#version 430 core\n"
	"out vec4 FragColor;\n";


//header for ambient light fragment shader
auto FRAGMENT_SHADER_HEADER_AMBIENT_LIGHT =
	"#define AMBIENT_LIGHT\n"
	"#define AMBIENT_LIGHT_SAMPLES 3\n"
	"uniform vec3 u_ambient_light_colors[AMBIENT_LIGHT_SAMPLES];\n"
	"uniform float u_ambient_light_colors_sample_positions[AMBIENT_LIGHT_SAMPLES];\n";


//header for direct light fragment shader
auto FRAGMENT_SHADER_HEADER_DIRECT_LIGHT =
	"#define DIRECT_LIGHT\n"
	"uniform sampler2D direct_light_map_texture;\n"
	"layout (std140,  binding = 1) uniform DIRECT_LIGHT_UNIFORMS\n"
	"{\n"
	"	vec3 direct_light_direction;\n"
	"	float direct_light_intensity;\n"
	"	vec3 direct_light_color;\n"
	"	float direct_light_light_angle;\n"
	"	mat4 direct_light_light_space_matrix;\n"
	"};\n";
