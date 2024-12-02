#include "ObjectInfo.h"

ObjectInfo::ObjectInfo(Scene* scene)
{
	scene_ = scene;
}

void ObjectInfo::gui_tick()
{
	std::string name = "Name: ";
	float pos[3] = {0,0,0};
	float rot[3] = {0,0,0};
	bool disabled;
	if (scene_->has_selected_object())
	{
		disabled = false;

		name+= scene_->get_selected_object()->name;
		pos[0] = scene_->get_selected_object()->getWorldPosition().x;
		pos[1] = scene_->get_selected_object()->getWorldPosition().y;
		pos[2] = scene_->get_selected_object()->getWorldPosition().z;

		rot[0] = scene_->get_selected_object()->getLocalRotationDegrees().x;
		rot[1] = scene_->get_selected_object()->getLocalRotationDegrees().y;
		rot[2] = scene_->get_selected_object()->getLocalRotationDegrees().z;
	} else
	{
		name+= "None selected";
		disabled  =true;
	}
	
	ImGui::Begin("Object info", &is_active, ImGuiWindowFlags_MenuBar);
	ImGui::Text(name.c_str());
	ImGui::SeparatorText("Transform");
	ImGui::BeginDisabled(disabled);
	ImGui::InputFloat3("WS Position",pos);
	ImGui::InputFloat3("Rotation",rot);

	
	if (scene_->has_selected_object())
	{
		//draw modifier info
		scene_->get_selected_object()->draw_modifier_ui();
	}
	
	ImGui::EndDisabled();
	ImGui::End();

	if (scene_->has_selected_object())
	{
		scene_->get_selected_object()->set_position_global(pos[0],pos[1],pos[2]);
		scene_->get_selected_object()->setRotationLocalDegrees(rot[0],rot[1],rot[2]);
	}


}
