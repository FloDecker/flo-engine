#pragma once
#include <chrono>
#include <vec3.hpp>

#include "../Core/CommonDataStructures/StructBoundingBox.h"
#include "../Core/CommonDataStructures/ray_cast_result.h"

//ray trace acceleration structures

//if child_0 = -1 -> child_1 indicates the position of the found collider in the sceneColliders array


class Scene;
class collider_modifier;

struct kdTreeElement
{
	int child_0;
	int child_1;
	StructBoundingBox bb;
};

class StackedBB
{
public:
	explicit StackedBB(Scene* scene, std::vector<collider_modifier*> leafs);
	StackedBB(Scene* scene);

	void insert_leaf_node(collider_modifier* leaf);
	bool contains_leaf_node(const collider_modifier* leaf) const;

	void recalculate();
	kdTreeElement* get_scene_bb_entry_element() const;
	kdTreeElement* get_scene_bb_element(unsigned int id) const;
	collider_modifier* get_scene_bb_element_leaf(const kdTreeElement* leaf_node) const;
	static bool is_bb_element_leaf_node(const kdTreeElement* leaf_node);

	//traversal functions

	void scene_geometry_proximity_check(
		const glm::vec3& proximity_center,
		float radius, ray_cast_result* result
	);

	void scene_geometry_raycast(const glm::vec3& ray_start, const glm::vec3& ray_direction, ray_cast_result* result, float ray_length, Object3D* ignore = nullptr, bool
	                            ignore_back_face = true);

private:
	void calculateSceneTree();

	Scene* scene_;

	void recurse_proximity_check_bb_tree(
		const kdTreeElement* bb_to_check, const glm::vec3& proximity_center,
		float radius, ray_cast_result* result
	);

	void recursive_scene_geometry_raycast(const ::kdTreeElement* bb_to_check, const glm::vec3& ray_start,
	                                      const glm::vec3& ray_direction, ray_cast_result* result, float ray_length, Object3D* ignore, bool ignore_back_face =
		                                      true);


	kdTreeElement* axis_aligned_bb_tree_ = nullptr;
	int scene_bb_entry_id_ = -1;
	std::vector<collider_modifier*> leaf_nodes;
};
