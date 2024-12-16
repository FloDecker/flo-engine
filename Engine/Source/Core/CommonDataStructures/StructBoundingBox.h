#pragma once
#include <glm.hpp>

struct StructBoundingBox
{
    glm::vec3 min;
    glm::vec3 max;
};

struct StructCenteredBoundingBox
{
    glm::vec3 center;
    glm::vec3 scale;
};


struct StructBoundingSphere
{
    glm::vec3 center;
    float radius;
};
