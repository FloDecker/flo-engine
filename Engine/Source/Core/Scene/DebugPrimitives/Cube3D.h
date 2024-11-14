#pragma once
#include "../Object3D.h"
#include "../../Renderer/Primitives/Cube.h"

class Cube3D: public Object3D
{
public:
    explicit Cube3D(Object3D *parent);
    int drawSelf() override;
    glm::vec3 color = {1,0,0};
private:
    Cube *cube_; 

};
