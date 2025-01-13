#include <iostream>
#include "Object3D.h"

#include "imgui.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/string_cast.hpp"
#include "Scene.h"

Object3D::Object3D(Object3D* parent)
{
	this->parent = parent;
	this->scene_ = parent->scene_;
	this->global_context_ = parent->global_context_;
	parent->children.push_back(this);
}

Object3D::Object3D()
{
}

int Object3D::draw_(struct RenderContext* parentRenderContext)
{
	//set global  uniforms
	renderContext = parentRenderContext;
	if (!visible) return 0;
	if (parentRenderContext->pass == render_pass_lighting)
	{
		this->draw_self_shadow_pass();
	} else if (parentRenderContext->pass == render_pass_custom)
	{
		this->draw_self_custom_pass(parentRenderContext->custom_shader);
	}
	else
	{
		this->drawSelf();
	}
	
	for (auto& child : children)
	{
		child->draw_(parentRenderContext);
	}
	return 1;
}

void Object3D::addChild(Object3D* child)
{
	printf("Add child is not implemented!!");
	//if (child->parent)
	//{
	//    std::cerr << "Child already has parent" << std::endl;
	//    //TODO: allow reparenting
	//    return;
	//}
	//
	////TODO : maybe there is a more elegant solution for this
	//auto pos_global = child->getWorldPosition();
	//this->children.push_back(child);
	//child->parent = this;
	//child->set_position_global(pos_global);
}

Object3D* Object3D::get_parent() const
{
	return parent;
}

int Object3D::drawSelf()
{
	return 0;
}

int Object3D::draw_self_shadow_pass()
{
	return 0;
}

int Object3D::draw_self_custom_pass(ShaderProgram* shader_program)
{
	return 0;
}

void Object3D::on_transform_changed()
{
}

//getter

glm::mat4 Object3D::getGlobalTransform()
{
	return transformGlobal;
}

glm::mat4 Object3D::getGlobalTransformInverse()
{
	return global_transform_inverse_;
}

glm::mat3 Object3D::get_global_rotation_matrix() const
{
	auto m_rot = glm::mat3(transformLocal);
	m_rot[0] = normalize(m_rot[0]);
	m_rot[1] = normalize(m_rot[1]);
	m_rot[2] = normalize(m_rot[2]);
	return m_rot;
}


glm::vec3 Object3D::getWorldPosition()
{
	glm::vec4 temp = transformGlobal * glm::vec4(0, 0, 0, 1);
	return {temp.x, temp.y, temp.z};
}

glm::vec3 Object3D::getLocalRotation()
{
	//auto a = glm::eulerAngles(rotation_quat_);
	//return glm::vec3(glm::pitch(rotation_quat_), glm::yaw(rotation_quat_), glm::roll(rotation_quat_));
	//return a;
	return rotation_;
}


glm::vec3 Object3D::getLocalRotationDegrees()
{
	return degrees(getLocalRotation());
}

glm::vec3 Object3D::getForwardVector()
{
	glm::vec4 temp = transformGlobal * glm::vec4(forwardVectorLocal, 0);
	return normalize(glm::vec3(temp.x, temp.y, temp.z));
}

glm::vec3 Object3D::getRightVector()
{
	glm::vec4 temp = transformGlobal * glm::vec4(rightVectorLocal, 0);
	return normalize(glm::vec3(temp.x, temp.y, temp.z));
}

glm::vec3 Object3D::get_scale()
{
	return scale_;
}

glm::quat Object3D::get_quaternion_rotation() const
{
	return rotation_quat_;
}

Scene* Object3D::get_scene() const
{
	return scene_;
}

std::vector<Object3D*>& Object3D::get_children()
{
	return children;
}

Object3D* Object3D::get_child_by_tag(std::string* tag)
{
	for (auto child : children)
	{
		if (child->has_tag(*tag)) return child;
	}
	return nullptr;
}

void Object3D::ui_get_scene_structure_recursively(ImGuiTreeNodeFlags flags)
{
	if (IGNORE_IN_SCENE_TREE_VIEW) return;
	if (ImGui::TreeNodeEx((this->name.empty()) ? "..." : this->name.c_str(), flags))
	{
		if (ImGui::IsItemClicked())
		{
			scene_->select_object(this);
		}
		for (auto& child : this->children)
		{
			child->ui_get_scene_structure_recursively(flags);
		}
		ImGui::TreePop();
	}
}

void Object3D::draw_object_specific_ui()
{
}

//setter for transform


void Object3D::setPositionLocal(float x, float y, float z)
{
	setPositionLocal(glm::vec3(x, y, z));
}

void Object3D::move_local(glm::vec3 movement_vector)
{
	setPositionLocal(position_ + movement_vector);
}

void Object3D::setPositionLocal(glm::vec3 pos)
{
	position_ = pos;
	recalculate_local_transform();
}

void Object3D::setRotationLocal(float x, float y, float z)
{
	setRotationLocal(glm::vec3(x, y, z));
}

void Object3D::setRotationLocal(glm::quat quat)
{
	this->rotation_quat_ = normalize(quat);
	recalculate_local_transform();
}

void Object3D::setRotationLocalDegrees(glm::vec3 rotation)
{
	setRotationLocal(radians(rotation));
}

void Object3D::setRotationLocalDegrees(float x, float y, float z)
{
	setRotationLocalDegrees(glm::vec3(x, y, z));
}

void Object3D::setRotationLocal(glm::vec3 rotation)
{
	rotation.x = glm::clamp(rotation.x, -glm::pi<float>() / 2.0f, glm::pi<float>() / 2.0f);
	// Y
	auto q_y = angle_utils::vector_rotation_to_quat(vec_y, rotation.y);
	// X
	auto q_x = angle_utils::vector_rotation_to_quat(vec_x, rotation.x);
	// Z
	auto q_z = angle_utils::vector_rotation_to_quat(vec_z, rotation.z);

	this->rotation_ = rotation;
	this->rotation_quat_ = glm::quat(rotation); //q_z * q_y * q_x;
	//printf("[%f %f %f] -> [%f %f %f]\n", rotation.x, rotation.y, rotation.z, a.x, a.y, a.z);

	recalculate_local_transform();
}

void Object3D::setScale(float scale)
{
	setScale(glm::vec3(scale));
}

void Object3D::setScale(float x, float y, float z)
{
	setScale(glm::vec3(x, y, z));
}

void Object3D::setScale(glm::vec3 scale)
{
	scale_ = scale;
	recalculate_local_transform();
}

void Object3D::set_position_global(const glm::vec3& pos)
{
	const auto transform_inverse = inverse(parent->getGlobalTransform());
	const auto pos_new = transform_inverse * glm::vec4(pos, 1);
	auto scale_inverse = new glm::mat4(1.0);
	setPositionLocal(glm::vec3(pos_new * scale(*scale_inverse, glm::vec3(1, 1, 1) / scale_)));
	free(scale_inverse);
}

void Object3D::set_position_global(float x, float y, float z)
{
	set_position_global(glm::vec3(x, y, z));
}

//TODO : this can be optimized a lot
void Object3D::move_global(float x, float y, float z)
{
	move_global(glm::vec3(x, y, z));
}

void Object3D::move_global(const glm::vec3& movement_vector)
{
	const auto transform_inverse = inverse(parent->getGlobalTransform());
	move_local(transform_inverse * glm::vec4(movement_vector, 0));
}


void Object3D::add_modifier(modifier* modifier)
{
	this->modifiers_.push_back(modifier);
}

void Object3D::draw_modifier_ui() const
{
	for (auto modifier : modifiers_)
	{
		modifier->draw_gui();
	}
}

glm::vec3 Object3D::transform_vertex_to_world_space(const glm::vec3& vertex_in_local_space) const
{
	return transformGlobal * glm::vec4(vertex_in_local_space, 1);
}

glm::vec3 Object3D::transform_position_to_local_space(const glm::vec3& vertex_in_world_space)
{
	glm::vec4 ray_cast_origin_vec4_local = getGlobalTransformInverse() * glm::vec4(vertex_in_world_space, 1);
	return glm::vec3(ray_cast_origin_vec4_local);
}

glm::vec3 Object3D::transform_vector_to_local_space(const glm::vec3& vector_in_world_space)
{
	glm::vec4 ray_cast_origin_vec4_local = getGlobalTransformInverse() * glm::vec4(vector_in_world_space, 0);
	return glm::vec3(ray_cast_origin_vec4_local);
}


void Object3D::recalculate_local_transform()
{
	transformLocal = glm::mat4(1.0f);

	//apply scale
	transformLocal = scale(transformLocal, scale_);

	//apply transform
	transformLocal = translate(transformLocal, position_);

	//apply rotation
	//transformLocal = glm::rotate(transformLocal, rotation_.y, vec_y);
	//transformLocal = glm::rotate(transformLocal, rotation_.x, vec_x);
	//transformLocal = glm::rotate(transformLocal, rotation_.z, vec_z);

	transformLocal = transformLocal * toMat4(rotation_quat_);
	this->recalculate_global_transform();
}

void Object3D::recalculate_global_transform()
{
	this->transformGlobal = (parent) ? parent->transformGlobal * this->transformLocal : this->transformLocal;
	this->global_transform_inverse_ = inverse(this->transformGlobal);
	for (auto child : this->children)
	{
		child->recalculate_global_transform();
	}
	on_transform_changed();
}


//tags

void Object3D::add_tag(std::string tag)
{
	auto tag_id = scene_->get_global_context()->tag_manager.get_id_of_tag(tag);
	if (tag_id < 0)
	{
		std::cout << "tag: " << tag.c_str() << " cant be added since its not part of the Tagmanager";
		return;
	}

	tags_.push_back(tag_id);
}

void Object3D::remove_tag(std::string tag)
{
	unsigned int tag_id = scene_->get_global_context()->tag_manager.get_id_of_tag(tag);
	if (tag_id < 0)
	{
		std::cout << "tag: " << tag.c_str() << " cant be removed since its not part of the Tagmanager";
		return;
	}
	if (!has_tag(tag_id))
	{
		std::cout << "tag: " << tag.c_str() << " this object (" << name << ") doesn't contain tag" << tag.c_str();
		return;
	}
	for (int i = 0; i < tags_.size(); i++)
	{
		if (tags_[i] == tag_id)
		{
			tags_.erase(tags_.begin() + i);
			return;
		}
	}
}

bool Object3D::has_tag(const unsigned tag_id) const
{
	for (const auto tag : tags_)
	{
		if (tag == tag_id)
		{
			return true;
		}
	}
	return false;
}

bool Object3D::has_tag(const std::string& tag) const
{
	const auto id = scene_->get_global_context()->tag_manager.get_id_of_tag(tag);
	if (id < 0)
	{
		return false;
	}

	return has_tag(id);
}
