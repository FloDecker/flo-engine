#include "gui_scene_tools.h"

#include "imgui.h"
#include "../Scene/Scene.h"

void gui_scene_tools::gui_tick()
{
	ImGui::Begin("Debug tools", &is_active, ImGuiWindowFlags_MenuBar);
	if (ImGui::Button("Recalculate bounding boxes"))
	{
		scene_->recalculate_from_root();
	}

	ImGui::DragInt("Gaussian samples per object", &scene_->gaussian_samples_per_object);
	ImGui::End();
}
