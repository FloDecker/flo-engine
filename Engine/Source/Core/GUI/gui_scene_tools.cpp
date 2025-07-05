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

	if (ImGui::Button("Dispatch light approx compute shader"))
	{
		for (int j = 0; j < 10; j++)
		{
			for (int i = 0; i < 27; i++)
			{
				scene_->test_compute_shader_approxmiate_ao->use();
				scene_->test_compute_shader_approxmiate_ao->setUniformInt("offset_id", i);
				scene_->test_compute_shader_approxmiate_ao->setUniformInt("calculation_level", i);
				glDispatchCompute(16, 16, 16);
				//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			}
		}
	}
	
	scene_->get_surfel_manager()->draw_ui();
	ImGui::End();
}
