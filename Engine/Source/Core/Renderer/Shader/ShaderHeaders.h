#pragma once
#include <format>

const char* name_direct_light_texture_sampler = "direct_light_texture_sampler";
const char* name_direct_light_direction = "direct_light_direction";
const char* name_direct_light_intensity = "direct_light_intensity";
const char* name_direct_light_color = "direct_light_color";


const char* name_u_ambient_light_colors = "u_ambient_light_colors";
const char* name_ambient_light_colors_sample_positions = "u_ambient_light_colors_sample_positions";


//header for standard vertex shader
const char* VERTEX_SHADER_HEADER_BASE =
	"#version 420 core \n"
	"layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec3 aNormal;\n"
	"layout (location = 2) in vec2 aUV;\n"
	"uniform mat4 mMatrix;\n"
	"uniform mat4 vMatrix;\n"
	"uniform mat4 pMatrix;\n"
	"uniform vec3 cameraPosWS;\n";

//header for standard fragment shader
const char* FRAGMENT_SHADER_HEADER_BASE =
	"#version 420 core\n"
	"out vec4 FragColor;\n";



//header for ambient light fragment shader
const char* FRAGMENT_SHADER_HEADER_AMBIENT_LIGHT =
	"#define AMBIENT_LIGHT\n"
	"#define AMBIENT_LIGHT_SAMPLES 3\n"
	"uniform vec3 u_ambient_light_colors[AMBIENT_LIGHT_SAMPLES];\n"
	"uniform float u_ambient_light_colors_sample_positions[AMBIENT_LIGHT_SAMPLES];\n";


//header for direct light fragment shader
const char* FRAGMENT_SHADER_HEADER_DIRECT_LIGHT =
	"#define DIRECT_LIGHT\n"
	//"uniform vec3 direct_light_direction;\n"
	//"uniform vec3 direct_light_color;\n"
	//"uniform float direct_light_intensity;\n";
	"layout (std140,  binding = 1) uniform DIRECT_LIGHT_UNIFORMS\n"
	"{\n"
	"	vec3 direct_light_direction;\n"
	"	float direct_light_intensity;\n"
	"	vec3 direct_light_color;\n"
	"	float direct_light_light_angle;\n"
	"	//uniform sampler2D light_map_texture;\n"
	"};\n";
