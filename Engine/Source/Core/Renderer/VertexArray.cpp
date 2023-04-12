//
// Created by flode on 28/02/2023.
//
#include "VertexArray.h"
#include "gtx/string_cast.hpp"


VertexArray::VertexArray(std::vector<Vertex> *vertices, std::vector<unsigned int> *indices) {
    this->vertices = vertices;
    this->indices = indices;

}


int VertexArray::load() {
    if (loaded) {
        return 0;
    }
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    std::cout<<vertices->size()<<std::endl;
    glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof(Vertex), vertices->data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(int), indices->data(), GL_STATIC_DRAW);

    std::cout<<sizeof(Vertex)<<std::endl;
    std::cout<<8*sizeof(float)<<std::endl;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) 0);//position - vec 3
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) sizeof(glm::vec3));//normal   - vec 3
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) (sizeof(glm::vec3) * 2));//uv       - vec 2

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    loaded = true;
    return 1;
}

//draws object with currently used shader
int VertexArray::draw() {
    if (!loaded) {
        std::cerr << "Draw-call issued for vertex array that hasn't been loaded yet" << std::endl;
        return -1;
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices->size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    return 1;
}