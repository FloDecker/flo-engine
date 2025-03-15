#include "ObjectInfo.h"

ObjectInfo::ObjectInfo(Scene* scene)
{
	scene_ = scene;
}

void ObjectInfo::gui_tick()
{
	std::string name = "Name: ";
	float pos[3] = {0, 0, 0};
	float rot[3] = {0, 0, 0};
	float scale[3] = {0, 0, 0};
	bool disabled;
	if (scene_->has_selected_object())
	{
		disabled = false;

		name += scene_->get_selected_object()->name;
		pos[0] = scene_->get_selected_object()->getWorldPosition().x;
		pos[1] = scene_->get_selected_object()->getWorldPosition().y;
		pos[2] = scene_->get_selected_object()->getWorldPosition().z;

		rot[0] = scene_->get_selected_object()->getLocalRotationDegrees().x;
		rot[1] = scene_->get_selected_object()->getLocalRotationDegrees().y;
		rot[2] = scene_->get_selected_object()->getLocalRotationDegrees().z;

		scale[0] = scene_->get_selected_object()->get_scale().x;
		scale[1] = scene_->get_selected_object()->get_scale().y;
		scale[2] = scene_->get_selected_object()->get_scale().z;
	}
	else
	{
		name += "None selected";
		disabled = true;
	}

	ImGui::Begin("Object info", &is_active, ImGuiWindowFlags_MenuBar);
	ImGui::Text(name.c_str());
	ImGui::SeparatorText("Transform");
	ImGui::BeginDisabled(disabled);


	bool transform_changed = ImGui::InputFloat3("WS Position", pos, "%.3f", ImGuiInputTextFlags_CharsDecimal);
	transform_changed |= ImGui::InputFloat3("Rotation", rot, "%.3f", ImGuiInputTextFlags_CharsDecimal);
	transform_changed |= ImGui::InputFloat3("Scale", scale, "%.3f", ImGuiInputTextFlags_CharsDecimal);


	if (scene_->has_selected_object())
	{
		//draw object info info
		scene_->get_selected_object()->draw_object_specific_ui();
		//draw modifier info
		scene_->get_selected_object()->draw_modifier_ui();
	}

	ImGui::EndDisabled();
	ImGui::End();

	if (scene_->has_selected_object() && transform_changed)
	{
		scene_->get_selected_object()->set_position_global(pos[0], pos[1], pos[2]);
		scene_->get_selected_object()->setRotationLocalDegrees(rot[0], rot[1], rot[2]);
		scene_->get_selected_object()->setScale(scale[0], scale[1], scale[2]);
	}
}
