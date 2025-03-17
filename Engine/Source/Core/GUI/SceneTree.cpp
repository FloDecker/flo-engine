#include "SceneTree.h"


SceneTree::SceneTree(Scene* scene)
{
	scene_ = scene;
}

void SceneTree::gui_tick()
{
	//sceen tree
	ImGui::Begin("Scene Tree", &is_active, ImGuiWindowFlags_MenuBar);
	scene_->get_root()->ui_get_scene_structure_recursively(ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::End();
}
