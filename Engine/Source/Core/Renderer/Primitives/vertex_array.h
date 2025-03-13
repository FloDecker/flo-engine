//
// Created by flode on 28/02/2023.
//
#ifndef ENGINE_VERTEXARRAY_H
#define ENGINE_VERTEXARRAY_H


#include "primitive.h"
#include "../Shader/ShaderProgram.h"
#include <glm.hpp>
#include <vector>
#include "../../CommonDataStructures/StructVertexArray.h"


class vertex_array : public primitive {
private:
    struct_vertex_array vertex_array_;
    unsigned int VBO = 0;
    unsigned int VAO = 0;
    unsigned int EBO = 0;

private:
    bool loaded = false;
    GLenum mode_ = GL_FILL;
    
public:
    vertex_array(std::vector<vertex> *vertices, std::vector<unsigned int> *indices);
    int load() override;
    int draw() override;
    
    
    bool set_draw_mode(GLenum mode);
    struct_vertex_array* get_vertex_array();
    primitive_type get_primitive_type() override;

};
#endif //ENGINE_VERTEXARRAY_H
