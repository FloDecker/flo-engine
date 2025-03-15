#pragma once
#include "GUIBase.h"
#include "../Editor/GlobalContext.h"

class LogGUI : public GUIBase
{
public:
	explicit LogGUI(const GlobalContext* global_context);

protected:
	void gui_tick() override;

private:
	Logger* logger_;
};
