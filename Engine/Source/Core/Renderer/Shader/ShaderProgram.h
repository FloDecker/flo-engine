#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H
#include "GL/glew.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "../Texture.h"


class ShaderProgram
{
private:
    char* vertexShader_;
    char* fragmentShader_;
    unsigned int shaderProgram_; //ID of the shader programm
    bool compiled = false;
    

public:
    std::vector<Texture> textures;
    enum shaderType { NONE, FRAGMENT, VERTEX };
    void loadFromFile(std::string pathOfMaterial);
    void setShader(char* fragmentShader, char* vertexShader);
    int compileShader();
    unsigned int getShaderProgram();
    void use();
    void initTextureUnits();
    

    //uniforms
    void setUniformMatrix4(const GLchar* name, const GLfloat* value);
};

#endif //ENGINE_SHADERPROGRAM_H
