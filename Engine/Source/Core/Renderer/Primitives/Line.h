#pragma once
#include <GL/glew.h>

#include "primitive.h"
#include "../../CommonDataStructures/StructVertexArray.h"


class Line : public primitive
{
public:
    int load() override;
    int draw() override;
    static bool loaded;
    static unsigned int VBO;
    static unsigned int VAO;
private:
    vertex points_[2] = {
        {
            glm::vec3(0, 0, 0),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },
        {
            glm::vec3(0, 0, 1),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        }
    };
};
