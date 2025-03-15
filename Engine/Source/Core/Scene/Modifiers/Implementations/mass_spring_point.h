#pragma once
#include <glm.hpp>
#include <vector>

#include "physics_object_modifier.h"
#include "../modifier.h"

class PhysicsEngine;
class mass_spring_point;

struct spring
{
	mass_spring_point* point_1;
	mass_spring_point* point_2;
	float stiffness;
	float initial_length;
};


class mass_spring_point final : public physics_object_modifier
{
public:
	mass_spring_point(Object3D* parent_game_object_3d);
	void calculate_forces() override;
	void draw_gui() override;
	float damp = 0.0f;
};
