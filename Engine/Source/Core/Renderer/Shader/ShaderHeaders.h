#pragma once

//header for standard vertex shader
const char* VERTEX_SHADER_HEADER_BASE =
"#version 330 core \n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"layout (location = 2) in vec2 aUV;\n"
"uniform mat4 mMatrix;\n"
"uniform mat4 vMatrix;\n"
"uniform mat4 pMatrix;\n"
"uniform vec3 cameraPosWS;\n";

//header for standard fragment shader
const char* FRAGMENT_SHADER_HEADER_BASE =
"#version 330 core\n"
"out vec4 FragColor;\n";
