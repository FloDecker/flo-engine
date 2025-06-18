#pragma once
#include "AbstractShaderProgram.h"

class compute_shader: public AbstractShaderProgram
{
public:
	void loadFromFile(std::string pathOfMaterial) override;
	int compileShader(bool recompile) override;
private:
	std::string compute_shader_source_;
	unsigned int compute_shader_id;
};
