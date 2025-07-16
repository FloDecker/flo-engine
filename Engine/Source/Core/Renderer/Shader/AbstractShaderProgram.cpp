#include "AbstractShaderProgram.h"

#include <iostream>
#include <gtc/type_ptr.inl>

//TODO: this can also be added before compilation
void AbstractShaderProgram::addTexture(texture* texture, const GLchar* samplerName)
{
	use();
	setUniformInt(samplerName, textures.size());
	textures.emplace_back(texture, samplerName);
}


int AbstractShaderProgram::use()
{
	if (!compiled) return -1;
	initTextureUnits();
	glUseProgram(shaderProgram_);
	return 0;
}


unsigned int AbstractShaderProgram::getShaderProgram()
{
	return this->shaderProgram_;
}

bool AbstractShaderProgram::is_compiled()
{
	return compiled;
}

void AbstractShaderProgram::initTextureUnits()
{
	for (unsigned int i = 0; i < textures.size(); ++i)
	{
		textures.at(i).texture->use(i);
	}
}


void AbstractShaderProgram::addVoxelField(texture_3d* texture, const GLchar* samplerName)
{
	addTexture(texture, samplerName);
	//TODO change name depending on sampler name
	set_uniform_vec3_f("voxel_field_lower_left", value_ptr(texture->get_voxel_field_lower_left()));
	set_uniform_vec3_f("voxel_field_upper_right", value_ptr(texture->get_voxel_field_upper_right()));

	set_uniform_float("voxel_field_step_size", texture->get_step_size());

	setUniformInt("voxel_field_depth", texture->get_depth());
	setUniformInt("voxel_field_height", texture->get_height());
	setUniformInt("voxel_field_height", texture->get_height());
}



bool AbstractShaderProgram::recompile_if_changed()
{
	struct stat result;
	if (stat(material_path_.c_str(), &result) == 0)
	{
		auto mod_time = result.st_mtime;
		if (mod_time != mod_time_)
		{
			std::cout << "recompiling " << material_path_.c_str() << "\n";
			loadFromFile(material_path_);
			compileShader(true);
			//add textures
			use();
			for (int i = 0; i < textures.size(); ++i)
			{
				setUniformInt(textures.at(i).samplerName, i);
			}
			return true;
		}
	}
	return false;
}


//set uniforms

void AbstractShaderProgram::set_uniform_vec3_f(const GLchar* name, const GLfloat value[3])
{
	if (!compiled)
	{
		std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
	}
	GLint location = glGetUniformLocation(shaderProgram_, name);
	glUniform3fv(location, 1, value);
}

void AbstractShaderProgram::set_uniform_array_vec3_f(const GLchar* name, const std::vector<glm::vec3>* color_array)
{
	if (!compiled)
	{
		std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
	}

	GLint location = glGetUniformLocation(shaderProgram_, name);
	glUniform3fv(location, color_array->size(), value_ptr(color_array->at(0)));
}

void AbstractShaderProgram::setUniformMatrix4(const GLchar* name, const GLfloat* value)
{
	if (!compiled)
	{
		std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
	}
	GLint location = glGetUniformLocation(shaderProgram_, name);
	glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void AbstractShaderProgram::set_uniform_float(const GLchar* name, const GLfloat value)
{
	if (!compiled)
	{
		std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
	}
	GLint location = glGetUniformLocation(shaderProgram_, name);
	glUniform1f(location, value);
}

void AbstractShaderProgram::set_uniform_array_float(const GLchar* name, const std::vector<float>* float_array)
{
	if (!compiled)
	{
		std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
	}

	GLint location = glGetUniformLocation(shaderProgram_, name);
	glUniform1fv(location, float_array->size(), float_array->data());
}

void AbstractShaderProgram::setUniformInt(const GLchar* name, GLint value)
{
	if (!compiled)
	{
		std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
	}

	GLint location = glGetUniformLocation(shaderProgram_, name);
	glUniform1i(location, value);
}

void AbstractShaderProgram::set_uniform_vec3_u(const GLchar* name, const GLuint value[3])
{
	if (!compiled)
	{
		std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
	}
	GLint location = glGetUniformLocation(shaderProgram_, name);
	glUniform3uiv(location, 1, value);
}