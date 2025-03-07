#include "Handle.h"

#include "Scene.h"
#include "../../Util/RayIntersectionHelper.h"
#include "../../Core/CommonDataStructures/struct_intersection.h"
#include "../../Core/CommonDataStructures/ray_cast_result.h"
#include "../Scene/Modifiers/Implementations/Colliders/collider_modifier.h"

Handle::Handle(Scene* scene): Object3D(scene->get_root())
{
	//load handler models
	auto engine_handler_arrow_model = loadModel("EngineContent/Arrow.fbx");
	engine_handler_arrow_model->initializeVertexArrays();

	//load handler shader
	auto* handler_red = new ShaderProgram();
	handler_red->loadFromFile("EngineContent/Shader/HandlerRed.glsl");
	handler_red->compileShader();

	auto* handler_green = new ShaderProgram();
	handler_green->loadFromFile("EngineContent/Shader/HandlerGreen.glsl");
	handler_green->compileShader();

	auto* handler_blue = new ShaderProgram();
	handler_blue->loadFromFile("EngineContent/Shader/HandlerBlue.glsl");
	handler_blue->compileShader();

	std::string engine_collider_tag = "ENGINE_COLLIDER";
	std::string engine_handle_collider_tag = "ENGINE_HANDLE_COLLIDER";

	arrow_x = new Mesh3D(this, engine_handler_arrow_model);
	arrow_x->add_material(handler_red);
	arrow_x->setRotationLocalDegrees(0, 90, 0);

	arrow_y = new Mesh3D(this, engine_handler_arrow_model);
	arrow_y->add_material(handler_green);
	arrow_y->setRotationLocalDegrees(-90, 0, 0);

	arrow_z = new Mesh3D(this, engine_handler_arrow_model);
	arrow_z->add_material(handler_blue);

	arrow_x_collider_ = dynamic_cast<collider_modifier*>(arrow_x->get_modifiers_by_id(100)[0]);
	arrow_y_collider_ = dynamic_cast<collider_modifier*>(arrow_y->get_modifiers_by_id(100)[0]);
	arrow_z_collider_ = dynamic_cast<collider_modifier*>(arrow_z->get_modifiers_by_id(100)[0]);
	
	detach();

	IGNORE_IN_SCENE_TREE_VIEW = true;
}

void Handle::attach_to_object(Object3D* object_3d)
{
	visible = true;
	attached_ = true;
	attached_object_3d_ = object_3d;
	this->setPositionLocal(object_3d->getWorldPosition());
	last_pos_ = object_3d->getWorldPosition();
}

void Handle::detach()
{
	visible = false;
	attached_ = false;
	attached_object_3d_ = nullptr;
	handler_status = not_transforming;
}

bool Handle::is_attached() const
{
	return attached_;
}

bool Handle::is_moving_coord() const
{
	return handler_status != not_transforming;
}

void Handle::editor_click_handle(glm::vec3 camera_pos, glm::vec3 ray_direction)
{
	auto cast_hit = ray_cast_result();
	arrow_x_collider_->ray_intersection_world_space(camera_pos, ray_direction, 100000.0, true, &cast_hit);
	if (cast_hit.hit)
	{
		handler_status = move_global_x;
		offset_ = glm::vec3(cast_hit.hit_local.z,0,0);
		return;
	}

	arrow_y_collider_->ray_intersection_world_space(camera_pos, ray_direction, 100000.0, true, &cast_hit);
	if (cast_hit.hit)
	{
		handler_status = move_global_y;
		offset_ = glm::vec3(0,cast_hit.hit_local.z,0);
		return;
	}


	arrow_z_collider_->ray_intersection_world_space(camera_pos, ray_direction, 100000.0, true, &cast_hit);
	if (cast_hit.hit)
	{
		handler_status = move_global_z;
		offset_ = glm::vec3(0,0,cast_hit.hit_local.z);
	}
}

void Handle::editor_release_handle()
{
	handler_status = not_transforming;
}

void Handle::editor_move_handle(glm::vec3 camera_pos, glm::vec3 ray_direction)
{
	auto intersection = new struct_intersection;

	switch (handler_status)
	{
	case not_transforming:
		return;
	case move_global_x:
		RayIntersectionHelper::ray_plane_intersection(intersection, camera_pos, ray_direction,
		                                            attached_object_3d_->getWorldPosition(), vec_z);
		if (intersection->intersected)
		{
			auto current_object_pos = attached_object_3d_->getWorldPosition();
			current_object_pos.x = intersection->collision_point.x;
			auto pos = current_object_pos - offset_;
			attached_object_3d_->set_position_global(pos);
			this->set_position_global(pos);
		}
		return;
	case move_global_y:
		RayIntersectionHelper::ray_plane_intersection(intersection, camera_pos, ray_direction,
		                                            attached_object_3d_->getWorldPosition(), vec_z);
		if (intersection->intersected)
		{
			auto current_object_pos = attached_object_3d_->getWorldPosition();
			current_object_pos.y = intersection->collision_point.y;
			auto pos = current_object_pos - offset_;

			attached_object_3d_->set_position_global(pos);
			this->set_position_global(pos);
		}
		return;
	case move_global_z:
		RayIntersectionHelper::ray_plane_intersection(intersection, camera_pos, ray_direction,
		                                            attached_object_3d_->getWorldPosition(), vec_x);
		if (intersection->intersected)
		{
			auto current_object_pos = attached_object_3d_->getWorldPosition();
			current_object_pos.z = intersection->collision_point.z;
			auto pos = current_object_pos - offset_;

			attached_object_3d_->set_position_global(pos);
			this->set_position_global(pos);
		}
		return;
	}

	free(intersection);
}
