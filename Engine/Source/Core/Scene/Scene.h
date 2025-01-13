#pragma once

#include <unordered_set>

#include "Handle.h"
#include "Object3D.h"
#include "../Editor/GlobalContext.h"
#include "Lighting/PointLight.h"
#include "Lighting/direct_light.h"
#include "../../Util/StackedBB.h"
#include "../CommonDataStructures/StructColorRange.h"
#include "../CommonDataStructures/StructMeshTriangleFilter.h"
#include "DebugPrimitives/visual_debug_tools.h"

class sky_box;
class MeshCollider;

class SceneRoot : public Object3D
{
public:
	SceneRoot(GlobalContext* global_context, Scene* scene);
	int draw_entry_point(RenderContext* render_context) const;
};




//scene context contains scene specific information
class Scene
{
public:
	Scene(GlobalContext* global_context, const std::string& name);
	std::vector<PointLight*> get_scene_point_lights();
	direct_light *get_scene_direct_light() const;
	void recalculate_at(Object3D* parent);
	void recalculate_from_root();
	void calculateColliderBoundingBoxes();
	Object3D* get_root() const;
	GlobalContext* get_global_context() const;
	StackedBB* get_bb() const;
	visual_debug_tools* get_debug_tools() const;
	void draw_scene(RenderContext* render_context) const;
	void light_pass(camera* current_camera) const;
	void custom_pass(RenderContext* render_context) const;
	void select_object(Object3D* object);
	void deselect();

	std::vector<Collider*>* get_colliders_in_bounding_box(StructBoundingBox* bounding_box) const;

	std::vector<glm::vec3>* get_polygons_in_bounding_box(StructBoundingBox* bounding_box) const;

	std::vector<std::tuple<MeshCollider*, std::vector<vertex_array_filter>*>>* get_triangles_in_bounding_box(
		StructBoundingBox* bounding_box) const;

	Handle* handle() const;
	Object3D* get_selected_object() const;
	bool has_selected_object() const;

	//returns an approximation of the ao color at a given position in world space
	StructColorRange* get_ao_color_at(int samples, glm::vec3 ws_pos) const;

	//register scene objects
	void register_global_light(direct_light *direct_light);
	void register_sky_box(sky_box *skybox);

private:
	std::unordered_set<PointLight*> scenePointLights;
	std::vector<Collider*> sceneColliders;
	StackedBB* scene_bb;
	GlobalContext* global_context_;

	SceneRoot* scene_root_;

	//ids for engine defined tags
	unsigned int engine_light_point_id_;
	unsigned int engine_collider_id_;

	//name of the scene
	std::string name_;

	bool _has_selected_object = false;
	Object3D* selected_object = nullptr;

	Handle* handle_;

	visual_debug_tools* visual_debug_tools_;

	//direct light
	bool has_direct_light_ = false;
	direct_light *direct_light_ = nullptr;

	//skybox
	bool has_sky_box = false;
	sky_box *sky_box_ = nullptr;

	RenderContext *light_pass_render_context_;


};
