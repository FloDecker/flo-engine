#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H
#include "GL/glew.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "../Texture/Texture.h"
#include "../Texture/Texture3D.h"

struct RenderContext;
class Object3D;

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

enum shader_header_includes
{
    DEFAULT_HEADERS,
    DYNAMIC_DIRECTIONAL_LIGHT,
    DYNAMIC_AMBIENT_LIGHT
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

    //flags
    bool flag_include_default_header_ = true;
    bool flag_include_dynamic_directional_light_ = false;
    bool flag_include_dynamic_ambient_light_ = false;


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
    void add_header_uniforms(Object3D *object_3d, RenderContext* renderContext);


    void set_shader_header_include(shader_header_includes include, bool include_header);
    
    //returns true if recompiles
    bool recompile_if_changed();
    

    //uniforms
    void set_uniform_vec3_f(const GLchar* name, const GLfloat value[3]);
    void set_uniform_array_vec3_f(const GLchar* name, const std::vector<glm::vec3>* color_array);
    void setUniformMatrix4(const GLchar* name, const GLfloat* value);
    void set_uniform_float(const GLchar *name, const GLfloat value);
    void set_uniform_array_float(const GLchar *name, const std::vector<float>* float_array);
    void addTexture(Texture *texture, const GLchar *samplerName);
    void addVoxelField(Texture3D *texture, const GLchar *samplerName);
    void setUniformInt(const GLchar *name, GLint value);
};

#endif //ENGINE_SHADERPROGRAM_H
