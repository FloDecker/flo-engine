#pragma once
#include <vec3.hpp>

#include "../Modifier.h"

class PhysicsEngine;

class PhysicsObjectModifier: public Modifier
{
friend class PhysicsEngine; //direct access to acceleration and velocity members
public:
	PhysicsObjectModifier(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine);
	void move(glm::vec3 increment) const;
private:
	glm::vec3 acceleration_;
	glm::vec3 velocity_;
};
