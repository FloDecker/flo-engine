#pragma once
#include "../Object3D.h"

class PointLight: public Object3D
{
public:
    PointLight(Object3D *parent);
    float intensity;
    glm::vec3 color;
};
