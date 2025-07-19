#pragma once
#include <format>

auto name_direct_light_texture_sampler = "direct_light_texture_sampler";
auto name_direct_light_direction = "direct_light_direction";
auto name_direct_light_intensity = "direct_light_intensity";
auto name_direct_light_color = "direct_light_color";


auto name_u_ambient_light_colors = "u_ambient_light_colors";
auto name_ambient_light_colors_sample_positions = "u_ambient_light_colors_sample_positions";

auto VERSION_430 = "#version 430 core\n";


//header for standard vertex shader
auto VERTEX_SHADER_HEADER_BASE =
"#define DEFAULT_VERTEX_SHADER\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"layout (location = 2) in vec2 aUV;\n"
"uniform mat4 mMatrix;\n"
"uniform mat4 vMatrix;\n"
"uniform mat4 pMatrix;\n"
"uniform vec3 cameraPosWS;\n"
"out vec3 pos_ws;\n"
"out vec3 normal_ws;\n";

auto VERTEX_SHADER_FOOTER_BASE =
"void main() {\n"
"vec3 pos_u = aPos;\n"
"vec3 normal_u = aNormal;\n"
"main_u();\n"
"pos_ws = (mMatrix * vec4(pos_u, 1.0)).xyz;\n"
"normal_ws = (transpose(inverse(mMatrix)) * vec4(normal_u, 0.0)).xyz;\n"
"vec4 vertexCamSpace =vMatrix * mMatrix * vec4(pos_u, 1.0);\n"
"gl_Position = pMatrix * vertexCamSpace;\n"
"}\n";


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

//header for rendering a deferred pass
auto FRAGMENT_SHADER_HEADER_DEFERRED_PASS =
"#define DEFERRED\n"
"layout (location = 0) out vec3 gPosition;\n"
"layout (location = 1) out vec3 gNormal;\n"
"layout (location = 2) out vec4 gAlbedoSpec;\n"
"layout (location = 3) out uint gRenderFlags;\n"
"#ifdef DEFAULT_VERTEX_SHADER\n"
"in vec3 pos_ws;\n"
"in vec3 normal_ws;\n"

"#endif\n";
auto FRAGMENT_SHADER_FOOTHER_DEFERRED_PASS =
	"#ifdef DEFAULT_VERTEX_SHADER\n"
	"void main(){\n"
	"gPosition = pos_ws;\n"
	"gNormal = normal_ws;\n"
	"gAlbedoSpec = vec4(1,1,0,1);\n"
	"gRenderFlags = 0u;\n"
	"main_u();\n"
	"}\n"
	"#endif\n"
	"#ifndef DEFAULT_VERTEX_SHADER\n"
	"void main(){\n"
	"main_u();\n"
	"}\n"
	"#endif\n";


//header for standard fragment shader
auto FRAGMENT_SHADER_HEADER_FORWARD =
"#define FORWARD\n"
"out vec4 FragColor;\n"
"#ifdef DEFAULT_VERTEX_SHADER\n"
"in vec3 pos_ws;\n"
"in vec3 normal_ws;\n"
"#endif\n";

auto FRAGMENT_SHADER_FOOTER_FORWARD =
"#ifdef DEFAULT_VERTEX_SHADER\n"
"void main(){\n"
"	main_u();\n"
"}\n"
"#endif\n"
"#ifndef DEFAULT_VERTEX_SHADER\n"
"void main(){\n"
"	main_u();\n"
"}\n"
"#endif\n";

auto FRAGMENT_SHADER_HEADER_DEFINE_DEFAULT_VERTEX_SHADER =
	"#define DEFAULT_VERTEX_SHADER\n";