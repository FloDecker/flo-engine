#pragma once

enum collision_channel 
{
	VISIBILITY,
	PHYSICS
};

static constexpr collision_channel all_collision_channels[] = { VISIBILITY, PHYSICS};