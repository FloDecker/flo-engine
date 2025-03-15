#pragma once
#include "GUIBase.h"

class Scene;

class gui_scene_tools : public GUIBase
{
public:
	explicit gui_scene_tools(Scene* scene)
		: scene_(scene)
	{
	}

protected:
	void gui_tick() override;

private:
	Scene* scene_;
};
