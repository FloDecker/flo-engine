#pragma once
#include <map>

#include "GUIBase.h"

class performance_metrics;
class Scene;



class gui_performance_metrics : public GUIBase
{
private:
	performance_metrics* performance_metrics_;
protected:
	void gui_tick() override;

public:
	gui_performance_metrics(performance_metrics* performance_metrics);
};
