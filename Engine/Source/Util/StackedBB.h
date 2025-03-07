#pragma once
#include <chrono>
#include "../Core/CommonDataStructures/StructBoundingBox.h"
#include "../Core/CommonDataStructures/ray_cast_result.h"
//ray trace acceleration structures

//if child_0 = -1 -> child_1 indicates the position of the found collider in the sceneColliders array


class collider_modifier;
class Object3D;

struct kdTreeElement
{
	int child_0;
	int child_1;
	StructBoundingBox bb;
};

class StackedBB
{
public:
	StackedBB(std::vector<collider_modifier*> leafs);

	void recalculate();
	kdTreeElement* get_scene_bb_entry_element() const;
	kdTreeElement* get_scene_bb_element(unsigned int id) const;
	collider_modifier* get_scene_bb_element_leaf(const kdTreeElement* leaf_node) const;
	static bool is_bb_element_leaf_node(const kdTreeElement* leaf_node);

	//traversal functions

	ray_cast_result* scene_geometry_proximity_check(
		const glm::vec3& proximity_center,
		float radius
	);

private:

	
	void calculateSceneTree();


	ray_cast_result *recurse_proximity_check_bb_tree(
		const kdTreeElement* bb_to_check,const glm::vec3& proximity_center,
		float radius
	);

	
	kdTreeElement* axis_aligned_bb_tree_ = nullptr;
	int scene_bb_entry_id_ = -1;
	std::vector<collider_modifier*> leaf_nodes;
};
