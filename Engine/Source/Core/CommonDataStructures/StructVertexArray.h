#pragma once
#include <glm.hpp>
#include <vector>

struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

struct struct_vertex_array
{
    std::vector<vertex> *vertices; //an array of vertices 
    std::vector<unsigned int> *indices; //indexes which of the vertices from the vertex array from a face
};
