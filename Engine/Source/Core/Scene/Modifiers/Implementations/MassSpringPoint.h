#pragma once
#include <vec3.hpp>

#include "PhysicsObjectModifier.h"
#include "../Modifier.h"

class PhysicsEngine;

class MassSpringPoint : public PhysicsObjectModifier
{
public:
	MassSpringPoint(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine);
	
private:
	glm::vec3 force_;
	glm::vec3 velocity_;
	float mass_;
	float damping_;
};
