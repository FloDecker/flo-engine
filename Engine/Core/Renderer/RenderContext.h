
//
// Created by flode on 02/03/2023.
//

#ifndef ENGINE_RENDERCONTEXT_H
#define ENGINE_RENDERCONTEXT_H

#include "vec3.hpp"
#include "matrix_float4x4.hpp"
class Camera {
public:
    Camera(float width, float height);
    glm::mat4 *getProjection();
    glm::mat4 *getView();
    void setViewPortDimension(float width, float height);
    void setFOV(float FOV);
    void setClippingPlanes(float near, float far);
    void recalculateProjection(); //has to be called when camera changes
    void calculateView(glm::mat4 cameraTransform); //has to be called when camera changes position

    glm::mat4 view;
private:
    float FOV_ = 90.0;
    float nearClippingPlane_ = 0.01;
    float farClippingPlane_ = 1000.0;
    float height_;
    float width_;

    glm::vec3 pos;
    glm::vec3 rot;

    glm::mat4 projection;

};


struct RenderContext {
    Camera camera;
    double deltaTime = 0;
};

#endif //ENGINE_RENDERCONTEXT_H
