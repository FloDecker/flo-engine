//
// Created by flode on 28/02/2023.
//

#include "ShaderProgram.h"

#include <vec2.hpp>
#include <vec3.hpp>
#include <gtc/type_ptr.hpp>

#include "ShaderHeaders.h"


class Texture;
const char* vertexShaderTag   = "[vertex]";
const char* fragmentShaderTag = "[fragment]";

void ShaderProgram::createVertexShaderInstruction(std::string* strPointer) const
{
    strPointer->append(VERTEX_SHADER_HEADER_BASE);
    strPointer->append("\n");
    strPointer->append(this->vertexShader_);
}

void ShaderProgram::createFragmentShaderInstruction(std::string* strPointer) const
{
    strPointer->append(FRAGMENT_SHADER_HEADER_BASE);
    strPointer->append("\n");
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
    
    if(!materialFileStream.is_open())
    {
        return;
    }

    struct stat result;
    if(stat(pathOfMaterial.c_str(), &result)==0)
    {
        mod_time_ = result.st_mtime;
    }
    
    std::string line;
    while (materialFileStream.good()) {

        std::getline(materialFileStream,line);
        if (line == vertexShaderTag) {
            lastShaderTag = VERTEX;
            continue;
        }
        if (line==fragmentShaderTag) {
            lastShaderTag = FRAGMENT;
            continue;
        }
        switch (lastShaderTag) {
            
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

void ShaderProgram::setShader(char *fragmentShader, char *vertexShader) {
    this->fragmentShader_ = fragmentShader;
    this->vertexShader_ = vertexShader;
}

int ShaderProgram::compileShader(bool recompile) {
    if (compiled && ! recompile) return 0;
    compiled = false;

    
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    
    std::string vertexShaderComplete;
    this->createVertexShaderInstruction(&vertexShaderComplete);
    char* pVertex = (char*)(malloc(vertexShaderComplete.size() + 1));
    memcpy_s(pVertex,vertexShaderComplete.size()+1,vertexShaderComplete.data(),vertexShaderComplete.size()+1);
    glShaderSource(vertexShader,1,&pVertex,NULL);
    glCompileShader(vertexShader);
    
    int  success;
    char infoLog[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "Failed to compile vertex shader Log: " << infoLog << std::endl;
        std::cout<< "FAILED VERTEX SHADER:" << std::endl;
        std::cout<<pVertex<<std::endl;
        free(pVertex);
        return -1;
    }

    free(pVertex);


    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    std::string fragmentShaderComplete;
    this->createFragmentShaderInstruction(&fragmentShaderComplete);
    char* pFragment = static_cast<char*>(malloc(fragmentShaderComplete.size() + 1));
    memcpy_s(pFragment,fragmentShaderComplete.size()+1,fragmentShaderComplete.data(),fragmentShaderComplete.size()+1);
    glShaderSource(fragmentShader,1,&pFragment,NULL);
    glCompileShader(fragmentShader);


    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "Failed to compile fragment shader Log: " << infoLog << std::endl;
        std::cout<< "FAILED FRAGMENT SHADER:" << std::endl;
        std::cout<<pFragment<<std::endl;
        free(pFragment);
        return -1;
    }

    free(pFragment);

    if (!compiled)
    {
        this->shaderProgram_ = glCreateProgram();
    } 
    
    glAttachShader(this->shaderProgram_,vertexShader);
    glAttachShader(this->shaderProgram_,fragmentShader);
    glLinkProgram(this->shaderProgram_);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    compiled = true;
    return 1;
}

unsigned int ShaderProgram::getShaderProgram() {
    return this->shaderProgram_;
}

bool ShaderProgram::is_compiled()
{
    return compiled;
}

int ShaderProgram::use() {
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

//TODO: this can also be added before compilation
void ShaderProgram::addTexture(Texture* texture, const GLchar *samplerName)
{
    use();
    setUniformInt(samplerName,textures.size());
    textures.emplace_back(texture,samplerName);
}

void ShaderProgram::addVoxelField(Texture3D* texture, const GLchar* samplerName)
{
    addTexture(texture,samplerName);
    //TODO change name depending on sampler name
    set_uniform_vec3_f("voxel_field_lower_left",glm::value_ptr(texture->get_voxel_field_lower_left()));
    set_uniform_vec3_f("voxel_field_upper_right",glm::value_ptr(texture->get_voxel_field_upper_right()));
    
    set_uniform_float("voxel_field_step_size",texture->get_step_size());
   
    setUniformInt("voxel_field_depth",texture->get_depth());
    setUniformInt("voxel_field_height",texture->get_height());
    setUniformInt("voxel_field_height",texture->get_height());
}

void ShaderProgram::recompile_if_changed()
{
    struct stat result;
    if(stat(material_path_.c_str(), &result)==0)
    {
        auto mod_time = result.st_mtime;
        if (mod_time != mod_time_)
        {
            std::cout<<"recompiling "<<material_path_.c_str()<<"\n"; 
            loadFromFile(material_path_);
            compileShader(true);
        }
    }

    
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

void ShaderProgram::setUniformMatrix4(const GLchar *name, const GLfloat *value) {
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

void ShaderProgram::setUniformInt(const GLchar *name, GLint value)
{
    if (!compiled)
    {
        std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
    }
    
    GLint location = glGetUniformLocation(shaderProgram_, name);
    glUniform1i(location, value);
}

