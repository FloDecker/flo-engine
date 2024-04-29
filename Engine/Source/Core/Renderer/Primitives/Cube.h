#pragma once
#include <GL/glew.h>

#include "../renderable.h"
#include "../../CommonDataStructures/StructVertexArray.h"


class Cube : public Renderable
{

public:
    static bool loaded;
    static unsigned int VBO;
    static unsigned int VAO;

    int load() override;
    int draw() override;

private:
    vertex points_[24] = {
        //bottom
        {
            glm::vec3(-0.5, 0.5, -0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },
        {
            glm::vec3(-0.5, 0.5, 0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },


        {
            glm::vec3(-0.5, 0.5, 0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },
        {
            glm::vec3(0.5, 0.5, 0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },


        {
            glm::vec3(0.5, 0.5, 0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },
        {
            glm::vec3(0.5, 0.5, -0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },


        {
            glm::vec3(0.5, 0.5, -0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },
        {
            glm::vec3(-0.5, 0.5, -0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        }
        //middle
        ,
        {
            glm::vec3(0.5, 0.5, 0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },
        {
            glm::vec3(0.5, -0.5, 0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },

        {
            glm::vec3(-0.5, 0.5, 0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },
        {
            glm::vec3(-0.5, -0.5, 0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },

        {
            glm::vec3(0.5, 0.5, -0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },
        {
            glm::vec3(0.5, -0.5, -0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },


        {
            glm::vec3(-0.5, 0.5, -0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },
        {
            glm::vec3(-0.5, -0.5, -0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },

        //top
        {
            glm::vec3(-0.5, -0.5, -0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },
        {
            glm::vec3(-0.5, -0.5, 0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },


        {
            glm::vec3(-0.5, -0.5, 0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },
        {
            glm::vec3(0.5, -0.5, 0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },


        {
            glm::vec3(0.5, -0.5, 0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },
        {
            glm::vec3(0.5, -0.5, -0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },


        {
            glm::vec3(0.5, -0.5, -0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        },
        {
            glm::vec3(-0.5, -0.5, -0.5),
            glm::vec3(0, 0, 0),
            glm::vec2(0, 0),
        }

    };
};
