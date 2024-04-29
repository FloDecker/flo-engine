#include "Line3D.h"

#include <gtc/type_ptr.hpp>
#include <gtx/dual_quaternion.hpp>

Line3D::Line3D(Object3D* scene_root, glm::vec3 pos_0, glm::vec3 pos_1, GlobalContext* global_context): Object3D(
    global_context)
{
    this->pos_0 = pos_0;
    this->pos_1 = pos_1;
    scene_root->addChild(this);
    float distance = glm::distance(pos_0, pos_1);
    this->setScale(distance, distance, distance);
    this->set_position_global(pos_0);


    glm::vec3 v = glm::normalize(pos_1 - pos_0);
    double thetaX, thetaY, thetaZ;
    
    line_ = new Line();
    line_->load();
}

int Line3D::drawSelf()
{
    global_context_->default_color_debug_shader->use();
    global_context_->default_color_debug_shader->setUniformMatrix4("mMatrix", glm::value_ptr(this->transformGlobal));
    global_context_->default_color_debug_shader->setUniformMatrix4(
        "vMatrix", glm::value_ptr(*this->renderContext->camera.getView()));
    global_context_->default_color_debug_shader->setUniformMatrix4(
        "pMatrix", glm::value_ptr(*this->renderContext->camera.getProjection()));
    global_context_->default_color_debug_shader->setUniformVec3F("cameraPosWS",
                                                                 glm::value_ptr(
                                                                     *this->renderContext->camera.getWorldPosition()));
    global_context_->default_color_debug_shader->setUniformVec3F("color", glm::value_ptr(color));
    line_->draw();
    return 1;
}
