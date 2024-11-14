#pragma once
#include "../Object3D.h"
#include "../../Renderer/Primitives/Line.h"

class Line3D : public Object3D
{
public:
    explicit Line3D(Object3D *parent, glm::vec3 pos_0, glm::vec3 pos_1);
    int drawSelf() override;
    glm::vec3 pos_0;
    glm::vec3 pos_1;
    glm::vec3 color = {1,0,0};
    void set_positions( glm::vec3 pos_0, glm::vec3 pos_1);
private:
    Line *line_; 
};
