#pragma once
#include <vec3.hpp>
#include <vector>

#include "PhysicsObjectModifier.h"
#include "../Modifier.h"

class PhysicsEngine;
class mass_spring_point;
struct spring
{
	mass_spring_point *point_1;
	mass_spring_point *point_2;
	float stiffness;
	float initial_length;
};


class mass_spring_point : public PhysicsObjectModifier
{
public:
	mass_spring_point(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine);
	void calculate_forces() override;
	void draw_gui() override;
	float damp = 0.0f;
};

