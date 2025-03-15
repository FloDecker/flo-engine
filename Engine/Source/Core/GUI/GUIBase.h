#pragma once
#include <string>

class GUIBase
{
	friend class GUIManager;

protected:
	bool is_active = true;

	virtual void gui_tick()
	{
	};
};
