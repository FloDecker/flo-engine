#include "gui_scene_tools.h"

#include "imgui.h"
#include "../Renderer/Shader/compute_shader.h"
#include "../Scene/Scene.h"
#include "../Scene/SceneTools/SurfelManagerOctree.h"

void gui_scene_tools::gui_tick()
{
	ImGui::Begin("Debug tools", &is_active, ImGuiWindowFlags_MenuBar);
	if (ImGui::Button("Recalculate bounding boxes"))
	{
		scene_->recalculate_from_root();
	}


	
	scene_->get_surfel_manager()->draw_ui();
	ImGui::End();
}
