//
// Created by flode on 28/02/2023.
//
#include "renderable.h"
#include "Shader/ShaderProgram.h"
#include "type_ptr.hpp"
#include <glm.hpp>


#ifndef ENGINE_VERTEXARRAY_H
#define ENGINE_VERTEXARRAY_H
class VertexArray : public Renderable {

private:
    unsigned int lengthVertices;
    unsigned int lengthIndices;
    float *vertices;
    unsigned int *indices;
    ShaderProgram *shaderProgram;
    unsigned int VBO = 0;
    unsigned int VAO = 0;
    unsigned int EBO = 0;
    bool loaded = false;

public:
    VertexArray(unsigned int lengthVertices,unsigned int lengthIndices, float *vertices, unsigned int *indices, ShaderProgram *shaderProgram);
    int load() override;
    int draw() override;

    //temp uniforms
    glm::mat4 *model;
    glm::mat4 *view;
    glm::mat4 *projection;


};
#endif //ENGINE_VERTEXARRAY_H
