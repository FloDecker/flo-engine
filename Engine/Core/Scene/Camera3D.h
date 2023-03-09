//
// Created by flode on 04/03/2023.
//

#ifndef ENGINE_CAMERA3D_H
#define ENGINE_CAMERA3D_H

#include "../Renderer/RenderContext.h"
#include "Object3D.h"

class Camera3D : public Object3D {

private:
    RenderContext *renderContext_;

public:
    explicit Camera3D(RenderContext *renderContext);
    void setRenderContext(RenderContext *renderContext);
    RenderContext *getRenderContext();
    void calculateView();
protected:
    int drawSelf() override;
};


#endif //ENGINE_CAMERA3D_H
