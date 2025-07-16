#pragma once

#include "GL/glew.h"
#include <string>
#include <vec3.hpp>
#include <vector>
#include "../Texture/texture.h"
#include "../Texture/texture_3d.h"

struct RenderContext;
class Object3D;

struct Sampler
{
	Sampler(texture* texture, const GLchar* sampler_name)
		: texture(texture),
		  samplerName(sampler_name)
	{
	}

	texture* texture;
	const GLchar* samplerName;
};


class AbstractShaderProgram
{
public:
	std::vector<Sampler> textures;

	
	//uniforms
	void set_uniform_vec3_f(const GLchar* name, const GLfloat value[3]);
	void set_uniform_array_vec3_f(const GLchar* name, const std::vector<glm::vec3>* color_array);
	void setUniformMatrix4(const GLchar* name, const GLfloat* value);
	void set_uniform_float(const GLchar* name, GLfloat value);
	void set_uniform_array_float(const GLchar* name, const std::vector<float>* float_array);
	void addTexture(texture* texture, const GLchar* samplerName);
	void addVoxelField(texture_3d* texture, const GLchar* samplerName);
	void setUniformInt(const GLchar* name, GLint value);
	void set_uniform_vec3_u(const GLchar* name, const GLuint value[3]);


	//returns true if recompiles
	bool recompile_if_changed();
	unsigned int getShaderProgram();
	bool is_compiled();
	void initTextureUnits();
	int use();

	virtual void loadFromFile(std::string pathOfMaterial)
	{
	}

	virtual int compileShader(bool recompile) { return false; }
	
protected:
	bool compiled = false;
	time_t mod_time_ = 0;

	std::string material_path_;
	unsigned int shaderProgram_; //ID of the shader programm

};
