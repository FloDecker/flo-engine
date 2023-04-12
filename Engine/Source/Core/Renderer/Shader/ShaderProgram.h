//
// Created by flode on 28/02/2023.
//
#include "GL/glew.h"
#include <iostream>
#include <glm.hpp>

#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H
class ShaderProgram{
private:
    char *vertexShader_;
    char *fragmentShader_;
    unsigned int shaderProgram_; //ID of the shader programm
    bool compiled = false;

public:
    void setShader(char *fragmentShader, char* vertexShader);
    int compileShader();
    unsigned int getShaderProgram();
    void use() const;

    //uniforms
    void setUniformMatrix4(const GLchar * name, const GLfloat * value);
};


#endif //ENGINE_SHADERPROGRAM_H
