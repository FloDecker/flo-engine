//
// Created by flode on 28/02/2023.
//

#include "ShaderProgram.h"

#include <glm.hpp>
#include <gtc/type_ptr.hpp>

#include "ShaderHeaders.h"
#include "../../Scene/Object3D.h"
#include "../RenderContext.h"
#include "../../Scene/Scene.h"
#include "../../Scene/SceneTools/SurfelManagerUniformGrid.h"


class texture;
auto vertexShaderTag = "[vertex]";
auto fragmentShaderTag = "[fragment]";

void ShaderProgram::createVertexShaderInstruction(std::string* strPointer) const
{
	if (flag_include_default_header_)
	{
		strPointer->append(VERTEX_SHADER_HEADER_BASE);
		strPointer->append("\n");
	}
	strPointer->append(this->vertexShader_);
}

void ShaderProgram::createFragmentShaderInstruction(std::string* strPointer) const
{
	if (flag_include_default_header_)
	{
		strPointer->append(FRAGMENT_SHADER_HEADER_BASE);
		strPointer->append("\n");
	}

	if (flag_include_dynamic_directional_light_)
	{
		strPointer->append(FRAGMENT_SHADER_HEADER_DIRECT_LIGHT);
		strPointer->append("\n");
	}

	if (flag_include_dynamic_ambient_light_)
	{
		strPointer->append(FRAGMENT_SHADER_HEADER_AMBIENT_LIGHT);
		strPointer->append("\n");
	}

	strPointer->append(this->fragmentShader_);
}

void ShaderProgram::loadFromFile(std::string pathOfMaterial)
{
	material_path_ = pathOfMaterial;
	//read shader 
	std::ifstream materialFileStream;
	materialFileStream.open(pathOfMaterial);
	shaderType lastShaderTag = NONE;
	std::string vertexShader;
	std::string fragmentShader;

	if (!materialFileStream.is_open())
	{
		return;
	}

	struct stat result;
	if (stat(pathOfMaterial.c_str(), &result) == 0)
	{
		mod_time_ = result.st_mtime;
	}

	std::string line;
	while (materialFileStream.good())
	{
		std::getline(materialFileStream, line);
		if (line == vertexShaderTag)
		{
			lastShaderTag = VERTEX;
			continue;
		}
		if (line == fragmentShaderTag)
		{
			lastShaderTag = FRAGMENT;
			continue;
		}
		switch (lastShaderTag)
		{
		case NONE:
			break;
		case VERTEX:
			vertexShader.append(line);
			vertexShader.append("\n");
			break;
		case FRAGMENT:
			fragmentShader.append(line);
			fragmentShader.append("\n");

			break;
		}
	}

	materialFileStream.close();

	//char* pFrag = static_cast<char*>(malloc(fragmentShader.size()));
	//memcpy_s(pFrag,fragmentShader.size()+1,fragmentShader.data(),fragmentShader.size()+1);
	this->fragmentShader_ = fragmentShader;

	//char* pVertex = static_cast<char*>(malloc(vertexShader.size()));
	//memcpy_s(pVertex,vertexShader.size()+1,vertexShader.data(),vertexShader.size()+1);
	this->vertexShader_ = vertexShader;
}

void ShaderProgram::setShader(char* fragmentShader, char* vertexShader)
{
	this->fragmentShader_ = fragmentShader;
	this->vertexShader_ = vertexShader;
}

int ShaderProgram::compileShader(bool recompile)
{
	if (compiled && !recompile) return 0;


	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	std::string vertexShaderComplete;
	this->createVertexShaderInstruction(&vertexShaderComplete);
	auto pVertex = static_cast<char*>(malloc(vertexShaderComplete.size() + 1));
	memcpy_s(pVertex, vertexShaderComplete.size() + 1, vertexShaderComplete.data(), vertexShaderComplete.size() + 1);
	glShaderSource(vertexShader, 1, &pVertex, nullptr);
	glCompileShader(vertexShader);

	int success;
	char infoLog[512];

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
		std::cout << "Failed to compile vertex shader Log: " << infoLog << std::endl;
		std::cout << "FAILED VERTEX SHADER:" << std::endl;
		std::cout << pVertex << std::endl;
		free(pVertex);

		if (compiled)
		{
			glDeleteProgram(this->shaderProgram_);
		}
		compiled = false;

		return -1;
	}

	free(pVertex);


	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	std::string fragmentShaderComplete;
	this->createFragmentShaderInstruction(&fragmentShaderComplete);
	auto pFragment = static_cast<char*>(malloc(fragmentShaderComplete.size() + 1));
	memcpy_s(pFragment, fragmentShaderComplete.size() + 1, fragmentShaderComplete.data(),
	         fragmentShaderComplete.size() + 1);
	glShaderSource(fragmentShader, 1, &pFragment, nullptr);
	glCompileShader(fragmentShader);


	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
		std::cout << "Failed to compile fragment shader Log: " << infoLog << std::endl;
		std::cout << "FAILED FRAGMENT SHADER:" << std::endl;
		std::cout << pFragment << std::endl;
		free(pFragment);

		if (compiled)
		{
			glDeleteProgram(this->shaderProgram_);
		}
		compiled = false;

		return -1;
	}
	free(pFragment);


	if (!compiled)
	{
		//compiling for the first time
		this->shaderProgram_ = glCreateProgram();
	}
	else
	{
		//recompiling
		glDetachShader(shaderProgram_, fragmentShaderID_);
		glDetachShader(shaderProgram_, vertexShaderID_);
	}

	fragmentShaderID_ = fragmentShader;
	vertexShaderID_ = vertexShader;

	glAttachShader(this->shaderProgram_, vertexShader);
	glAttachShader(this->shaderProgram_, fragmentShader);
	glLinkProgram(this->shaderProgram_);

	GLint isLinked = 0;
	glGetProgramiv(this->shaderProgram_, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE)
	{
		glGetProgramInfoLog(this->shaderProgram_, 512, nullptr, infoLog);
		std::cout << "Failed to link shader " << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	compiled = true;
	return 1;
}

unsigned int ShaderProgram::getShaderProgram()
{
	return this->shaderProgram_;
}

bool ShaderProgram::is_compiled()
{
	return compiled;
}

int ShaderProgram::use()
{
	if (!compiled) return -1;
	initTextureUnits();
	glUseProgram(shaderProgram_);
	return 0;
}

void ShaderProgram::initTextureUnits()
{
	for (unsigned int i = 0; i < textures.size(); ++i)
	{
		textures.at(i).texture->use(i);
	}
}

void ShaderProgram::add_header_uniforms(Object3D* object_3d, RenderContext* renderContext)
{
	if (flag_include_default_header_)
	{
		this->setUniformMatrix4("mMatrix", value_ptr(object_3d->getGlobalTransform()));
		this->setUniformMatrix4("vMatrix", value_ptr(*renderContext->camera->getView()));
		this->setUniformMatrix4("pMatrix", value_ptr(*renderContext->camera->getProjection()));
		this->set_uniform_vec3_f("cameraPosWS", value_ptr(*renderContext->camera->getWorldPosition()));
	}

	//if (flag_include_dynamic_directional_light_)
	//{
	//	this->set_uniform_float(name_direct_light_intensity, object_3d->get_scene()->get_scene_direct_light()->intensity);
	//}
	if (flag_include_dynamic_ambient_light_)
	{
		StructColorRange* color_range = object_3d->get_scene()->get_ao_color_at(0, glm::vec3(0, 0, 0));
		this->set_uniform_array_float(name_ambient_light_colors_sample_positions, &color_range->sample_points);
		this->set_uniform_array_vec3_f(name_u_ambient_light_colors, &color_range->colors);
	}
}

//TODO: this can also be added before compilation
void ShaderProgram::addTexture(texture* texture, const GLchar* samplerName)
{
	use();
	setUniformInt(samplerName, textures.size());
	textures.emplace_back(texture, samplerName);
}


void ShaderProgram::addVoxelField(texture_3d* texture, const GLchar* samplerName)
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

void ShaderProgram::set_shader_header_include(shader_header_includes include, bool include_header)
{
	switch (include)
	{
	case DEFAULT_HEADERS:
		flag_include_default_header_ = include_header;
		break;
	case DYNAMIC_DIRECTIONAL_LIGHT:
		flag_include_dynamic_directional_light_ = include_header;
		break;
	case DYNAMIC_AMBIENT_LIGHT:
		flag_include_dynamic_ambient_light_ = include_header;
		break;
	case GAUSSIAN_LIGHTING:
		flag_gaussian_lighting_ = include_header;
		break;
	}
	if (compiled) compileShader(true);
}

bool ShaderProgram::recompile_if_changed()
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

void ShaderProgram::set_uniform_vec3_f(const GLchar* name, const GLfloat value[3])
{
	if (!compiled)
	{
		std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
	}
	GLint location = glGetUniformLocation(shaderProgram_, name);
	glUniform3fv(location, 1, value);
}

void ShaderProgram::set_uniform_array_vec3_f(const GLchar* name, const std::vector<glm::vec3>* color_array)
{
	if (!compiled)
	{
		std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
	}

	GLint location = glGetUniformLocation(shaderProgram_, name);
	glUniform3fv(location, color_array->size(), value_ptr(color_array->at(0)));
}

void ShaderProgram::setUniformMatrix4(const GLchar* name, const GLfloat* value)
{
	if (!compiled)
	{
		std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
	}
	GLint location = glGetUniformLocation(shaderProgram_, name);
	glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void ShaderProgram::set_uniform_float(const GLchar* name, const GLfloat value)
{
	if (!compiled)
	{
		std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
	}
	GLint location = glGetUniformLocation(shaderProgram_, name);
	glUniform1f(location, value);
}

void ShaderProgram::set_uniform_array_float(const GLchar* name, const std::vector<float>* float_array)
{
	if (!compiled)
	{
		std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
	}

	GLint location = glGetUniformLocation(shaderProgram_, name);
	glUniform1fv(location, float_array->size(), float_array->data());
}

void ShaderProgram::setUniformInt(const GLchar* name, GLint value)
{
	if (!compiled)
	{
		std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
	}

	GLint location = glGetUniformLocation(shaderProgram_, name);
	glUniform1i(location, value);
}
