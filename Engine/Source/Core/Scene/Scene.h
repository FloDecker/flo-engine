#pragma once

#include <unordered_set>

#include "Handle.h"
#include "Object3D.h"
#include "../Editor/GlobalContext.h"
#include "Lighting/PointLight.h"
#include "Lighting/direct_light.h"
#include "../../Util/StackedBB.h"
#include "../CommonDataStructures/collider_intersection.h"
#include "../CommonDataStructures/StructColorRange.h"
#include "../CommonDataStructures/StructMeshTriangleFilter.h"
#include "../CommonDataStructures/collision_channel.h"
#include "../CommonDataStructures/surfel_irradiance_information.h"
#include "DebugPrimitives/visual_debug_tools.h"

class SurfelManagerOctree;
class texture_buffer_object;
struct surfel;
class mesh_collider;
class collider_modifier;
class PhysicsEngine;
class sky_box;


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
	direct_light* get_scene_direct_light() const;
	void recalculate_at(Object3D* parent);
	void recalculate_from_root();
	Object3D* get_root() const;
	GlobalContext* get_global_context() const;
	PhysicsEngine* get_physics_engine() const;

	StackedBB* get_bb(collision_channel c) const;

	visual_debug_tools* get_debug_tools() const;


	//render passes
	void draw_scene(RenderContext* render_context) const;
	void draw_debug_tools(const RenderContext* render_context) const;
	void light_pass(camera* current_camera) const;
	void custom_pass(RenderContext* render_context) const;

	//this is called after drawing the current frame
	void post_draw() const;


	void select_object(Object3D* object);
	void deselect();

	void get_colliders_in_bounding_box(StructBoundingBox* bounding_box,
	                                   collision_channel channel, std::vector<collider_modifier*>* result);

	//NOT IMPLEMENTED
	std::vector<glm::vec3>* get_polygons_in_bounding_box(StructBoundingBox* bounding_box) const;

	//NOT IMPLEMENTED
	std::vector<std::tuple<mesh_collider*, std::vector<vertex_array_filter>*>>* get_triangles_in_bounding_box(
		StructBoundingBox* bounding_box) const;

	//handle
	Handle* handle() const;
	Object3D* get_selected_object() const;
	bool has_selected_object() const;

	//returns an approximation of the ao color at a given position in world space
	StructColorRange* get_ao_color_at(int samples, glm::vec3 ws_pos) const;


	//register scene objects
	void register_global_light(direct_light* direct_light);
	void register_sky_box(sky_box* skybox);

	//register collider
	void register_collider(collider_modifier* collider);

	//COLLISIONS AND INTERSECTIONS

	void recalculate_collision_channel_bb_hierarchy(collision_channel channel) const;

	//get colliders by collision channel
	std::vector<collider_modifier*> get_colliders(collision_channel collision_channel);
	std::vector<collider_intersection> generate_overlaps_in_channel(collision_channel channel);

	//ray trace in scene (theses always work independent of the bb tree 
	ray_cast_result ray_cast_in_scene_unoptimized(glm::vec3 origin, glm::vec3 direction, float max_distance,
	                                              collision_channel collision_channel);
	ray_cast_result ray_cast_in_scene(glm::vec3 origin, glm::vec3 direction, float max_distance,
	                                  collision_channel collision_channel, Object3D* ignore = nullptr);

	surfel_irradiance_information get_irradiance_information(glm::vec3 pos_ws, glm::vec3 normal_ws, int primary_rays,
	                                                         float disc_radius = 0.0f);
	static glm::vec3 uniformHemisphereSample(glm::vec3 normal);
	static glm::vec2 uniformDiscSample(float radius);
	static glm::vec3 uniformDiscSample_ws(glm::vec3 pos, glm::vec3 normal, float radius);

	SurfelManagerOctree* get_surfel_manager() const;

private:
	std::unordered_set<PointLight*> scenePointLights;
	std::vector<collider_modifier*> colliders_;
	SurfelManagerOctree* surfel_manager_;

	std::map<collision_channel, StackedBB*> scene_bb;
	GlobalContext* global_context_;
	PhysicsEngine* physics_engine_;

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
	direct_light* direct_light_ = nullptr;


	//skybox
	bool has_sky_box = false;
	sky_box* sky_box_ = nullptr;

	RenderContext* light_pass_render_context_;
};

inline SurfelManagerOctree* Scene::get_surfel_manager() const
{
	return surfel_manager_;
}
