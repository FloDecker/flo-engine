//
// Created by flode on 28/02/2023.
//

#include "ShaderProgram.h"
const char* vertexShaderBase = "#version 330 core\n"
                               "layout (location = 0) in vec3 aPos;\n"
                               "uniform mat4 pMatrix;\n";

void ShaderProgram::setShader(char *fragmentShader, char *vertexShader) {
    this->fragmentShader_ = fragmentShader;
    this->vertexShader_ = vertexShader;
}

int ShaderProgram::compileShader() {
    if (compiled) return 0;
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader,1,&this->vertexShader_,NULL);
    glCompileShader(vertexShader);

    int  success;
    char infoLog[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "Failed to compile vertex shader Log: " << infoLog << std::endl;
        exit(-1);
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader,1,&this->fragmentShader_,NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "Failed to compile vertex shader Log: " << infoLog << std::endl;
        exit(-1);
    }

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


