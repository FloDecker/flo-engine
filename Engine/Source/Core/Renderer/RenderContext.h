
//
// Created by flode on 02/03/2023.
//

#ifndef ENGINE_RENDERCONTEXT_H
#define ENGINE_RENDERCONTEXT_H

#include "glm.hpp"

#include "vec3.hpp"

//camera logic

class Camera {
public:
    Camera(float width, float height);
    glm::mat4 *getProjection();
    glm::mat4 *getView();
    void setViewPortDimension(float width, float height);
    void setFOV(float FOV);
    void setClippingPlanes(float near, float far);
    void recalculateProjection(); //has to be called when camera changes
    void calculateView(glm::mat4 cameraTransform, glm::vec3 cameraPos, glm::vec3 cameraViewDirection); //has to be called when camera changes position
    glm::vec3 *getWorldPosition();
    glm::mat4 view;
    
private:
    float FOV_ = 90.0;
    float nearClippingPlane_ = 0.01;
    float farClippingPlane_ = 1000.0;
    float height_;
    float width_;

    
    glm::vec3 viewDirection_;
    glm::vec3 positionWS_;

    glm::mat4 projection;

};

//holds the information needed to render the current frame 
struct RenderContext {
    Camera camera;
    double deltaTime = 0;
};

#endif //ENGINE_RENDERCONTEXT_H
