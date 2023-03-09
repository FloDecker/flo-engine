//
// Created by flode on 28/02/2023.
//

#include "Object3D.h"
#include "../Renderer/VertexArray.h"

#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H
class Mesh: public Object3D {
private :
    VertexArray *vertexArray_;
protected:
    int drawSelf() override;
public:
    void setVertexArray(VertexArray *vertexArray);
};
#endif //ENGINE_MESH_H
