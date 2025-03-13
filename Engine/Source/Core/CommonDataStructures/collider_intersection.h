#pragma once
#include <vec3.hpp>

#include "struct_intersection.h"

class collider_modifier;

struct collider_intersection
{
	struct_intersection collision = {};
	collider_modifier* collider_a = nullptr;
	collider_modifier* collider_b = nullptr;
};
