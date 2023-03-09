//
// Created by flode on 28/02/2023.
//
#include "VertexArray.h"
#include "gtx/string_cast.hpp"


VertexArray::VertexArray(unsigned int lengthVertices,unsigned int lengthIndices, float *vertices, unsigned int *indices, ShaderProgram *shaderProgram) {
    this->lengthVertices = lengthVertices;
    this->lengthIndices = lengthIndices;
    this->vertices = vertices;
    this->indices = indices;
    this->shaderProgram = shaderProgram;
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
    glBufferData(GL_ARRAY_BUFFER, lengthVertices * sizeof(float), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lengthIndices * sizeof(int), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) nullptr);
    glEnableVertexAttribArray(0);
    loaded = true;
    return 1;
}

int VertexArray::draw() {
    if (!loaded) {
        std::cerr << "Draw-call issued for vertex array that hasn't been loaded yet" << std::endl;
        return -1;
    }
    glUseProgram(shaderProgram->getShaderProgram());
    //position
    unsigned int modelLoc = glGetUniformLocation(shaderProgram->getShaderProgram(), "mMatrix");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram->getShaderProgram(), "vMatrix");
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram->getShaderProgram(), "pMatrix");
    if (model) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(*model));
    } else {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));
    }
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(*view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*projection));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, lengthIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    return 1;
}




