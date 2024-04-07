#pragma once
#include "../Object3D.h"

class PointLight: public Object3D
{
public:
    PointLight(GlobalContext *global_context);
    float intensity;
    glm::vec3 color;
};
