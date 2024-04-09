//
// Created by flode on 28/02/2023.
//

#include "Object3D.h"
#include "../Renderer/VertexArray.h"
#include "../../Content/Mesh.h"

#ifndef ENGINE_MESH3D_H
#define ENGINE_MESH3D_H

//represents a 3D mesh

class Mesh3D: public Object3D {
private :
    Mesh *mesh;
protected:
    int drawSelf() override;
public:
    explicit Mesh3D(Mesh *mesh, GlobalContext *global_context);
    bool receive_light = true;
};
#endif //ENGINE_MESH3D_H
