//
// Created by flode on 28/02/2023.
//

#include "ShaderProgram.h"

#include <vec2.hpp>
#include <vec3.hpp>

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

int ShaderProgram::compileShader() {
    if (compiled) return 0;

    
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
        exit(-1);
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

        exit(-1);
    }

    free(pFragment);

    this->shaderProgram_ = glCreateProgram();
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

void ShaderProgram::use() {
    initTextureUnits();
    glUseProgram(shaderProgram_);
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
    setInt(samplerName,textures.size());
    textures.emplace_back(texture,samplerName);
}



//set uniforms

void ShaderProgram::setUniformVec3F(const GLchar* name, const GLfloat value[3])
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

void ShaderProgram::setInt(const GLchar *name, GLint value)
{
    if (!compiled)
    {
        std::cerr << "shader needs to be compiled before assigning uniforms" << std::endl;
    }
    
    GLint location = glGetUniformLocation(shaderProgram_, name);
    glUniform1i(location, value);
}

