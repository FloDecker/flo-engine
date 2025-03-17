#pragma once

enum collision_channel
{
	VISIBILITY,
	PHYSICS,
	HANDLER, //ENGINE ONLY
};

static constexpr collision_channel all_collision_channels[] = {VISIBILITY, PHYSICS};
