#include "LogGUI.h"

#include "imgui.h"

LogGUI::LogGUI(const GlobalContext* global_context)
{
	logger_ = global_context->logger;
}

void LogGUI::gui_tick()
{
	ImGui::Begin("Logs", &is_active, ImGuiWindowFlags_MenuBar);

	// Create a child region with a specific size and enable scrolling
	ImGui::BeginChild("ScrollingRegion", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_HorizontalScrollbar);
	for (int i = 1; i <= logger_->get_current_log_amount(); i++)
	{
		auto l = logger_->get_log_entries()->at(logger_->get_current_log_amount() - i);
		ImGui::Text("%s : %s", to_string(l.type), l.log.c_str());
	}

	ImGui::EndChild(); // End the scrollable child region

	ImGui::End();
}
