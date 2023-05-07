#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H
#include "GL/glew.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vec3.hpp>
#include <vector>

#include "../Texture.h"

struct Sampler
{
    Sampler(Texture* texture, const GLchar *sampler_name)
        : texture(texture),
          samplerName(sampler_name)
    {
    }

    Texture *texture;
    const  GLchar *samplerName;
};

class ShaderProgram
{
private:
    std::string vertexShader_;
    std::string fragmentShader_;
    
    unsigned int shaderProgram_; //ID of the shader programm
    bool compiled = false;
    void createVertexShaderInstruction(std::string *strPointer) const;
    void createFragmentShaderInstruction(std::string *strPointer) const;
    

public:
    enum shaderType { NONE, FRAGMENT, VERTEX };

    
    std::vector<Sampler> textures;
    void loadFromFile(std::string pathOfMaterial);
    void setShader(char* fragmentShader, char* vertexShader);
    int compileShader();
    unsigned int getShaderProgram();
    void use();
    void initTextureUnits();
    void addTexture(Texture *texture, const GLchar *samplerName);
    

    //uniforms
    void setUniformVec3F(const GLchar* name, const GLfloat value[3]);
    void setUniformMatrix4(const GLchar* name, const GLfloat* value);
    void setInt(const GLchar *name, GLint value);
};

#endif //ENGINE_SHADERPROGRAM_H
