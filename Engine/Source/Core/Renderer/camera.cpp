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

void camera::set_render_target(framebuffer_object* target)
{
	camera_render_target_ = target;
	has_attached_render_target_ = true;
}

framebuffer_object* camera::get_render_target() const
{
	return camera_render_target_;
}

void camera::use() const
{
	if (has_attached_render_target_)
	{
		camera_render_target_->render_to_framebuffer();
	}
}


void camera::setViewPortDimension(float width, float height)
{
	
	this->width_ = std::max(1.0f,width);
	this->height_ = std::max(1.0f,height);
	recalculateProjection();
	//update render target if attached
	if (has_attached_render_target_)
	{
		camera_render_target_->resize_attach_textures(static_cast<unsigned int>(width),
		                                              static_cast<unsigned int>(height));
		
	}
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
	view = inverse(cameraTransform);
	positionWS_ = cameraPos;
	viewDirection_ = cameraViewDirection;
}
