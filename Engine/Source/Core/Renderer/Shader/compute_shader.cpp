#include "compute_shader.h"
#include <fstream>
#include <iostream>

void compute_shader::loadFromFile(std::string pathOfMaterial)
{
	material_path_ = pathOfMaterial;
	//read shader 
	std::ifstream materialFileStream;
	materialFileStream.open(pathOfMaterial);
	compute_shader_source_ = "";

	if (!materialFileStream.is_open())
	{
		return;
	}

	struct stat result;
	if (stat(pathOfMaterial.c_str(), &result) == 0)
	{
		mod_time_ = result.st_mtime;
	}
	
	if (materialFileStream.peek() == 0xEF) {
		// Skip UTF-8 BOM
		char bom[3];
		materialFileStream.read(bom, 3);
	}
	
	
	std::string line;
	while (materialFileStream.good())
	{
		std::getline(materialFileStream, line);
		compute_shader_source_.append(line);
		compute_shader_source_.append("\n");

	}
}

int compute_shader::compileShader(bool recompile)
{
	if (compiled && !recompile) return 0;


	unsigned int compute_shader;
	compute_shader = glCreateShader(GL_COMPUTE_SHADER);

	auto p_compute_shader = static_cast<char*>(malloc(compute_shader_source_.size() + 1));
	memcpy_s(p_compute_shader, compute_shader_source_.size() + 1, compute_shader_source_.data(), compute_shader_source_.size() + 1);
	glShaderSource(compute_shader, 1, &p_compute_shader, nullptr);
	glCompileShader(compute_shader);

	int success;
	char infoLog[512];
	
	glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(compute_shader, 512, nullptr, infoLog);
		std::cout << "Failed to compile compute shader Log: " << infoLog << std::endl;
		std::cout << "FAILED COMPUTE SHADER:" << std::endl;
		std::cout << p_compute_shader << std::endl;
		free(p_compute_shader);

		if (compiled)
		{
			glDeleteProgram(this->shaderProgram_);
		}
		compiled = false;

		return -1;
	}

	free(p_compute_shader);
	
	if (!compiled)
	{
		//compiling for the first time
		this->shaderProgram_ = glCreateProgram();
	}
	else
	{
		//recompiling
		glDetachShader(shaderProgram_, compute_shader_id);
	}

	compute_shader_id = compute_shader;

	glAttachShader(this->shaderProgram_, compute_shader);
	glLinkProgram(this->shaderProgram_);

	GLint isLinked = 0;
	glGetProgramiv(this->shaderProgram_, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE)
	{
		glGetProgramInfoLog(this->shaderProgram_, 512, nullptr, infoLog);
		std::cout << "Failed to link shader " << infoLog << std::endl;
	}

	glDeleteShader(compute_shader);
	compiled = true;
	return 1;
}
