#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H
#include "GL/glew.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vec3.hpp>
#include <vector>

#include "../Texture/Texture.h"
#include "../Texture/Texture3D.h"

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
    std::string material_path_;
    
    unsigned int shaderProgram_; //ID of the shader programm
    unsigned int vertexShaderID_;
    unsigned int fragmentShaderID_;
    bool compiled = false;
    void createVertexShaderInstruction(std::string *strPointer) const;
    void createFragmentShaderInstruction(std::string *strPointer) const;

    time_t mod_time_ = 0; 
    

public:
    enum shaderType { NONE, FRAGMENT, VERTEX };

    std::vector<Sampler> textures;
    void loadFromFile(std::string pathOfMaterial);
    void setShader(char* fragmentShader, char* vertexShader);
    int compileShader(bool recompile = false);
    unsigned int getShaderProgram();
    bool is_compiled();
    int use();
    void initTextureUnits();
    void addTexture(Texture *texture, const GLchar *samplerName);
    void addVoxelField(Texture3D *texture, const GLchar *samplerName);

    //returns true if recompiles
    bool recompile_if_changed();
    

    //uniforms
    void set_uniform_vec3_f(const GLchar* name, const GLfloat value[3]);
    void setUniformMatrix4(const GLchar* name, const GLfloat* value);
    void set_uniform_float(const GLchar *name, const GLfloat value);

    void setUniformInt(const GLchar *name, GLint value);
};

#endif //ENGINE_SHADERPROGRAM_H
