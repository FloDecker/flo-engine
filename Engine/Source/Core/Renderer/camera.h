#pragma once
#include "framebuffer_object.h"
#include "glm.hpp"
#include "vec3.hpp"
#include "../../Engine/Source/External/eventpp/include/eventpp/eventqueue.h"


class camera
{
public:
	camera(float width, float height, eventpp::CallbackList<void (glm::ivec2)>* camera_change_dispatcher);
	glm::mat4* getProjection();
	glm::mat4* getView();
	void setViewPortDimension(glm::i16vec2 dimentions);
	void set_render_target(framebuffer_object* target);
	void use() const; //set this render target as active 
	void setViewPortDimension(float width, float height);
	void setFOV(float FOV);
	void setClippingPlanes(float near, float far);
	void recalculateProjection(); //has to be called when camera changes
	void calculateView(glm::mat4 cameraTransform, glm::vec3 cameraPos, glm::vec3 cameraViewDirection);
	//has to be called when camera changes position
	glm::vec3* getWorldPosition();
	glm::mat4 view;

private:
	float FOV_ = 90.0;
	float nearClippingPlane_ = 0.01;
	float farClippingPlane_ = 1000.0;
	float height_;
	float width_;

	//specifies the render target the camera is attached to
	bool has_attached_render_target_ = false;
	framebuffer_object* camera_render_target_ = nullptr;
	glm::vec3 viewDirection_;
	glm::vec3 positionWS_;

	glm::mat4 projection;
};
