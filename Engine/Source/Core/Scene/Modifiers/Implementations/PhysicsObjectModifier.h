#pragma once
#include <vec3.hpp>

#include "../Modifier.h"
#include "../../../PhysicsEngine/IntegrationMethods/Integrator.h"

class PhysicsEngine;

class PhysicsObjectModifier: public Modifier
{
public:
	PhysicsObjectModifier(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine);
	virtual void calculate_forces();
	void clear_force();
	void add_force(glm::vec3 force);
	void integrate_velocity(integrator *integrator, float delta);
	void integrate_position(integrator *integrator, float delta) const;
	void move(glm::vec3 increment) const;
private:
	glm::vec3 force_ = glm::vec3(0, 0, 0);
	glm::vec3 acceleration_ = glm::vec3(0, 0, 0);
	glm::vec3 velocity_ = glm::vec3(0, 0, 0);
	bool is_fixed_ = false;
};
