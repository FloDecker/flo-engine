#include "GUIManager.h"

void GUIManager::addGUI(GUIBase* gui)
{
	GUIs.push_back(gui);
}

void GUIManager::tickGUI() const
{
	for (GUIBase* gui : GUIs)
	{
		gui->gui_tick();
	}
	
}
