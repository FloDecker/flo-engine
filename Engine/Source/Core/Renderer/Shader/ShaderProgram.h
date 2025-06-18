#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H

#include "AbstractShaderProgram.h"


enum shader_header_includes
{
	DEFAULT_HEADERS,
	DYNAMIC_DIRECTIONAL_LIGHT,
	DYNAMIC_AMBIENT_LIGHT,
	GAUSSIAN_LIGHTING,
};

class ShaderProgram: public AbstractShaderProgram
{
	std::string vertexShader_;
	std::string fragmentShader_;

	unsigned int vertexShaderID_;
	unsigned int fragmentShaderID_;
	
	void createVertexShaderInstruction(std::string* strPointer) const;
	void createFragmentShaderInstruction(std::string* strPointer) const;


	//todo: may rework this
	//flags
	bool flag_include_default_header_ = true;
	bool flag_include_dynamic_directional_light_ = false;
	bool flag_include_dynamic_ambient_light_ = false;
	bool flag_gaussian_lighting_ = false;

public:
	enum shaderType { NONE, FRAGMENT, VERTEX };

	void loadFromFile(std::string pathOfMaterial) override;
	int compileShader(bool recompile = false) override;

	void setShader(char* fragmentShader, char* vertexShader);

	void add_header_uniforms(Object3D* object_3d, RenderContext* renderContext);


	void set_shader_header_include(shader_header_includes include, bool include_header);


	bool receives_dynamic_directional_light() const
	{
		return flag_include_dynamic_directional_light_;
	};


};

#endif //ENGINE_SHADERPROGRAM_H
