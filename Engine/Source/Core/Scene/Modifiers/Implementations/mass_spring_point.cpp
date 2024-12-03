#include "mass_spring_point.h"

#include "imgui.h"

mass_spring_point::mass_spring_point(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine):
	physics_object_modifier(parent_game_object_3d, physics_engine)
{
}

void mass_spring_point::calculate_forces()
{
	add_force(-get_velocity()*damp);
}

void mass_spring_point::draw_gui()
{
	physics_object_modifier::draw_gui();
	ImGui::InputFloat("Damp",&this->damp);

}

