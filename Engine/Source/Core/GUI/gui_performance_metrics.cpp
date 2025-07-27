#include "gui_performance_metrics.h"

#include "imgui.h"
#include "../Editor/performance_metrics.h"

gui_performance_metrics::gui_performance_metrics(performance_metrics* performance_metrics): performance_metrics_(
	performance_metrics)
{
}

void gui_performance_metrics::gui_tick()
{

	ImGui::Begin("Performance", &is_active, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginTable("performance table", 3, ImGuiTableFlags_Borders)) {
		// ticks
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Tick time in seconds");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text(performance_metrics_->get_metric_as_string(tick_cycle_time).c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::Text(performance_metrics_->get_average_metric_as_string(tick_cycle_time).c_str());


		// ticks
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("FPS");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text(std::to_string(1.0/performance_metrics_->get_metric(tick_cycle_time)).c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::Text(std::to_string(1.0/performance_metrics_->get_average_metric(tick_cycle_time)).c_str());
		
		// physics
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Physics");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text(std::to_string(performance_metrics_->get_metric(physics)).c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::Text(std::to_string(performance_metrics_->get_average_metric(physics)).c_str());
				
		// main
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Main pass");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text(std::to_string(performance_metrics_->get_metric(pass_main)).c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::Text(std::to_string(performance_metrics_->get_average_metric(pass_main)).c_str());
						
		// g buffer
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("G buffer and post processing");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text(std::to_string(performance_metrics_->get_metric(pass_g_buffer)).c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::Text(std::to_string(performance_metrics_->get_average_metric(pass_g_buffer)).c_str());
								
		// surfel pass
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Surfel pass");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text(std::to_string(performance_metrics_->get_metric(pass_surfels)).c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::Text(std::to_string(performance_metrics_->get_average_metric(pass_surfels)).c_str());
										
		// pass direct light
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Direct light pass");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text(std::to_string(performance_metrics_->get_metric(pass_direct_light)).c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::Text(std::to_string(performance_metrics_->get_average_metric(pass_direct_light)).c_str());

		// surfel tick
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Surfel tick");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text(std::to_string(performance_metrics_->get_metric(surfels_tick)).c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::Text(std::to_string(performance_metrics_->get_average_metric(surfels_tick)).c_str());
		
		ImGui::EndTable();
	}
	ImGui::End();

}
