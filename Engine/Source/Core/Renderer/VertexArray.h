//
// Created by flode on 28/02/2023.
//
#include "renderable.h"
#include "Shader/ShaderProgram.h"
#include <glm.hpp>
#include <vector>


#ifndef ENGINE_VERTEXARRAY_H
#define ENGINE_VERTEXARRAY_H

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class VertexArray : public Renderable {
private:
    std::vector<Vertex> *vertices;
    std::vector<unsigned int> *indices;
    unsigned int VBO = 0;
    unsigned int VAO = 0;
    unsigned int EBO = 0;
    bool loaded = false;

public:
    VertexArray(std::vector<Vertex> *vertices, std::vector<unsigned int> *indices);
    int load() override;
    int draw() override;

    //temp uniforms
    glm::mat4 *model;
    glm::mat4 *view;
    glm::mat4 *projection;


};
#endif //ENGINE_VERTEXARRAY_H
