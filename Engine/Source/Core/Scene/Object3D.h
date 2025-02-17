#pragma once
#include <glm.hpp>

#include <string>
#include <detail/type_quat.hpp>

#include "imgui.h"
#include "vector"
#include "../Editor/GlobalContext.h"
#include "../Renderer/RenderContext.h"
#include "Modifiers/modifier.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>
#include "../../Util/angle_utils.h"

//Forward declaration
class Scene;

constexpr auto vec_x = glm::vec3(1, 0, 0);
constexpr auto vec_y = glm::vec3(0, 1, 0);
constexpr auto vec_z = glm::vec3(0, 0, 1);

constexpr auto vec_x_w_vector = glm::vec4(1, 0, 0,0);
constexpr auto vec_y_w_vector = glm::vec4(0, 1, 0,0);
constexpr auto vec_z_w_vector = glm::vec4(0, 0, 1,0);

constexpr auto vec_x_w_point = glm::vec4(1, 0, 0,1);
constexpr auto vec_y_w_point = glm::vec4(0, 1, 0,1);
constexpr auto vec_z_w_point = glm::vec4(0, 0, 1,1);


//an object is the most basic type in a Scene

class Object3D
{
	friend class SceneRoot;

	Object3D(); //friend class SceneRoot overwrites this since it's the only Object3D without parent
	std::vector<unsigned int> tags_; //tags associated with this object
	std::vector<modifier*> modifiers_; //modifiers of this game object
	//transforms
	glm::vec3 rotation_ = {0.0, 0.0, 0.0};
	glm::quat rotation_quat_ = glm::identity<glm::quat>();
	glm::vec3 position_ = {0.0, 0.0, 0.0};
	glm::vec3 scale_ = {1.0, 1.0, 1.0};

	//hierarchy
	std::vector<Object3D*> children;
	Object3D* parent = nullptr;


	glm::mat4 global_transform_inverse_;


	int draw_(struct RenderContext* parentRenderContext);
	void recalculate_local_transform(); //should be called after changing location / scale / rotation
	void recalculate_global_transform();
	//called after recalculating local transform and calls child transform recalculation

	glm::vec3 transform_angles_engine_to_quat(glm::vec3 angles);
	glm::vec3 transform_angles_glm_to_engine(glm::vec3 angles);

	

public:
	Object3D(Object3D* parent);
	bool visible = true;
	std::string name;


	//TAGS
	void add_tag(std::string tag);
	void remove_tag(std::string tag);
	bool has_tag(unsigned int tag_id) const;
	bool has_tag(const std::string& tag) const;

	// transform local //
	void setPositionLocal(glm::vec3 pos);
	void setPositionLocal(float x, float y, float z);

	void move_local(glm::vec3 movement_vector);

	void setRotationLocal(glm::vec3 rotation);
	void setRotationLocal(float x, float y, float z);
	void setRotationLocal(glm::quat quat);

	void setRotationLocalDegrees(glm::vec3 rotation);
	void setRotationLocalDegrees(float x, float y, float z);
	void look_at_local(glm::vec3 pos_local_space);

	void setScale(float scale);
	void setScale(float x, float y, float z);
	void setScale(glm::vec3 scale);


	//transform global
	void set_position_global(const glm::vec3& pos);
	void set_position_global(float x, float y, float z);
	void move_global(const glm::vec3& pos);
	void move_global(float x, float y, float z);

	//Modifiers
	void add_modifier(modifier* modifier);
	void draw_modifier_ui() const;
	std::vector<modifier*> get_modifiers_by_id(int id) const;
	//TODO: make modifiers removable

	//Transform Helper

	//takes a vertex defined in the objects local space and returns the world space coordinates of it
	glm::vec3 transform_vertex_to_world_space(const glm::vec3& vertex_in_local_space) const;
	glm::vec3 transform_position_to_local_space(const glm::vec3& vertex_in_world_space);
	glm::vec3 transform_vector_to_local_space(const glm::vec3& vector_in_world_space);
	glm::vec3 transform_vector_to_world_space(const glm::vec3& vector_in_local_space) const;

	//Getter
	glm::mat4 getGlobalTransform();
	glm::mat4 getGlobalTransformInverse();
	glm::mat3 get_global_rotation_matrix() const;
	glm::vec3 getWorldPosition();
	glm::vec3 getLocalRotation();
	glm::mat3 getWorldRotation();
	glm::vec3 getLocalRotationDegrees();

	glm::vec3 getForwardVector();
	glm::vec3 getUpVector(); //TODO
	glm::vec3 getRightVector(); //TODO

	glm::vec3 get_scale();

	glm::quat get_quaternion_rotation() const;

	Scene * get_scene() const;

	////////////
	void addChild(Object3D* child);
	Object3D* get_parent() const;
	std::vector<Object3D*>& get_children();
	Object3D* get_child_by_tag(std::string* tag);

	//UI
	void ui_get_scene_structure_recursively(ImGuiTreeNodeFlags flags);
	virtual void draw_object_specific_ui();

protected:
	Scene* scene_;
	glm::mat4 transformLocal = glm::mat4(1.0f); //local transform
	glm::mat4 transformGlobal = glm::mat4(1.0f); //global transform is recalculated for each frame //TODO: may optimize
	struct RenderContext* renderContext;
	GlobalContext* global_context_;
	glm::vec3 forwardVectorLocal = glm::vec3(0, 0, 1);
	glm::vec3 upwardVectorLocal = glm::vec3(0, 1, 0);
	glm::vec3 rightVectorLocal = glm::vec3(1, 0, 0);
	virtual int drawSelf(); //implement drawing code here,its executed when traversing the scene tree every frame 
	virtual int draw_self_shadow_pass(); //called when running shadow pass
	virtual int draw_self_custom_pass(ShaderProgram *shader_program); //called when running custom pass
	virtual void on_transform_changed(); //called whenever the objects transforms change

	//FLAGS
	bool IGNORE_IN_SCENE_TREE_VIEW = false;
};
