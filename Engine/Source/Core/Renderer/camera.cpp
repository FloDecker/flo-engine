#include "camera.h"
#include "gtc/matrix_transform.hpp"

camera::camera(float width, float height, eventpp::CallbackList<void (glm::ivec2)>* camera_change_dispatcher)
{
	view = glm::mat4(1.0);
	setViewPortDimension(width, height);
	camera_change_dispatcher->append([this](glm::ivec2 i)
	{
		setViewPortDimension(i);
	});
	
	
}

void camera::setViewPortDimension(glm::i16vec2 dimentions)
{
	setViewPortDimension(dimentions.x, dimentions.y);
}


void camera::setViewPortDimension(float width, float height)
{
	this->width_ = width;
	this->height_ = height;
	recalculateProjection();
}

void camera::recalculateProjection()
{
	this->projection = glm::perspective(glm::radians(FOV_), this->width_ / this->height_, nearClippingPlane_,
										farClippingPlane_);
}

void camera::setClippingPlanes(float near, float far)
{
	nearClippingPlane_ = near;
	farClippingPlane_ = far;
	recalculateProjection();
}

void camera::setFOV(float FOV)
{
	FOV_ = FOV;
	recalculateProjection();
}


glm::mat4* camera::getProjection()
{
	return &projection;
}

glm::mat4* camera::getView()
{
	return &view;
}


glm::vec3* camera::getWorldPosition()
{
	return &positionWS_;
}


void camera::calculateView(glm::mat4 cameraTransform, glm::vec3 cameraPos, glm::vec3 cameraViewDirection)
{
	view = glm::inverse(cameraTransform);
	positionWS_ = cameraPos;
	viewDirection_ = cameraViewDirection;
}
