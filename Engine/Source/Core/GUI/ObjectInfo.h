#pragma once
#include "GUIBase.h"
#include "../Scene/Scene.h"


class ObjectInfo: public GUIBase
{
	
public:
	ObjectInfo(Scene *scene);

private:
	Scene* scene_;
	

protected:
	void gui_tick();
};


