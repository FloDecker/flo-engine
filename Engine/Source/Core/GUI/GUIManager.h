#pragma once
#include <vector>

#include "GUIBase.h"

class GUIManager
{
public:
	void addGUI(GUIBase* gui);
	void tickGUI() const;
private:
	std::vector<GUIBase*> GUIs;
	
};
