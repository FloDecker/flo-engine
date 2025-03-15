#pragma once
#include "GUIBase.h"
#include "../Scene/Scene.h"


class SceneTree : public GUIBase
{
public:
	SceneTree(Scene* scene);

private:
	Scene* scene_;

protected:
	void gui_tick() override;
};
