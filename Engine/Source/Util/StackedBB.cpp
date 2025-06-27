#include "StackedBB.h"
#include "BoundingBoxHelper.h"
#include "../../Core/Scene/Modifiers/Implementations/Colliders/mesh_collider.h"
#include "../Core/Scene/Scene.h"


StackedBB::StackedBB(Scene* scene, std::vector<collider_modifier*> leafs)
{
	leaf_nodes = leafs;
	scene_ = scene;
	calculateSceneTree();
}

StackedBB::StackedBB(Scene* scene)

{
	scene_ = scene;
	leaf_nodes = std::vector<collider_modifier*>();
}

void StackedBB::insert_leaf_node(collider_modifier* leaf)
{
	if (contains_leaf_node(leaf))
	{
		return;
	}
	leaf_nodes.push_back(leaf);
}

bool StackedBB::contains_leaf_node(const collider_modifier* leaf) const
{
	for (const auto l : leaf_nodes)
	{
		if (l == leaf)
		{
			return true;
		}
	}
	return false;
}

void StackedBB::recalculate()
{
	if (axis_aligned_bb_tree_)
	{
		free(axis_aligned_bb_tree_);
	}
	calculateSceneTree();
}

void StackedBB::calculateSceneTree()
{
	if (leaf_nodes.empty())return;

	auto start = std::chrono::system_clock::now();

	//allocate memory for kd tree
	//leaf_nodes.size() for the leaf nodes and leaf_nodes.size()-1 for the 
	axis_aligned_bb_tree_ = static_cast<kdTreeElement*>(calloc(leaf_nodes.size() * 2 - 1, sizeof(kdTreeElement)));

	//vector that holds the positions in axis_aligned_bb_tree_ that the matrix represents e.g at matrix x,y = 4 represents
	//the bounding box positioned at pos 8 in axis_aligned_bb_tree_ -> matrix_bb_tree_map[4] = 8
	std::vector<int> matrix_bb_tree_map;
	matrix_bb_tree_map.resize(leaf_nodes.size());

	//insert the objects into the leaf nodes
	for (unsigned int x = 0; x < leaf_nodes.size(); x++)
	{
		auto temp = kdTreeElement{
			-1,
			static_cast<int>(x),
			*leaf_nodes.at(x)->get_world_space_bounding_box()
		};
		axis_aligned_bb_tree_[x] = temp;
		matrix_bb_tree_map[x] = x;
	}

	StructBoundingBox temp_bb;
	BoundingBoxHelper::get_combined_bounding_box(&temp_bb, leaf_nodes.at(0)->get_world_space_bounding_box(),
	                                             leaf_nodes.at(1)->get_world_space_bounding_box());
	float smallest_box = BoundingBoxHelper::get_max_length_of_bb(&temp_bb);
	glm::i32vec2 next_merge = {0, 1};

	//calculate distance matrix
	std::vector<std::vector<float>> distance_matrix;
	distance_matrix.resize(leaf_nodes.size());

	for (unsigned int x = 0; x < leaf_nodes.size(); x++)
	{
		distance_matrix[x].resize(leaf_nodes.size());
		for (unsigned int y = x; y < leaf_nodes.size(); y++)
		{
			BoundingBoxHelper::get_combined_bounding_box(&temp_bb, leaf_nodes.at(x)->get_world_space_bounding_box(),
			                                             leaf_nodes.at(y)->get_world_space_bounding_box());
			float d = BoundingBoxHelper::get_max_length_of_bb(&temp_bb);
			distance_matrix[x][y] = d;
			if (d < smallest_box && x != y)
			{
				smallest_box = d;
				next_merge = {x, y};
			}
		}
	}

	//calculate distance matrix
	//combine the two closest and merge them
	for (unsigned int i = 0; i < leaf_nodes.size() - 1; i++)
	{
		int array_pos_of_next_merge_obj_0 = matrix_bb_tree_map[next_merge.x];
		int array_pos_of_next_merge_obj_1 = matrix_bb_tree_map[next_merge.y];
		temp_bb = StructBoundingBox{};
		BoundingBoxHelper::get_combined_bounding_box(
			&temp_bb,
			&axis_aligned_bb_tree_[array_pos_of_next_merge_obj_0].bb,
			&axis_aligned_bb_tree_[array_pos_of_next_merge_obj_1].bb);

		//merge the smallest box
		auto temp = kdTreeElement{
			array_pos_of_next_merge_obj_0,
			array_pos_of_next_merge_obj_1,
			temp_bb
		};


		//TODO REMOVE THIS TEST
		//auto c = new Cube3D(global_context_);
		//scene_root_->addChild(c);
		//c->setScale(BoundingBoxHelper::get_scale_of_bb(&temp_bb));
		//c->
		//c->set_position_global(BoundingBoxHelper::get_center_of_bb(&temp_bb));
		////////////////////TEST END //////////////////////////
		///
		glm::vec3 color = {1, static_cast<float>(i) / static_cast<float>(leaf_nodes.size()), 1};
		scene_->get_debug_tools()->draw_debug_cube(BoundingBoxHelper::get_center_of_bb(&temp_bb), 2, glm::quat(),
		                                           BoundingBoxHelper::get_scale_of_bb(&temp_bb), color);

		//add new bounding box containing both objects into the array
		axis_aligned_bb_tree_[leaf_nodes.size() + i] = temp;

		if (distance_matrix.size() <= 2)
		{
			scene_bb_entry_id_ = leaf_nodes.size() + i;
			break;
		}

		int larger_position_in_matrix = std::max(next_merge.x, next_merge.y);
		int smaller_position_in_matrix = std::min(next_merge.x, next_merge.y);

		//remove the last element
		//row
		distance_matrix.erase(distance_matrix.begin() + larger_position_in_matrix);
		//colum
		for (int x = 0; x < distance_matrix.size(); x++)
		{
			distance_matrix[x].erase(distance_matrix[x].begin() + larger_position_in_matrix);
		}

		matrix_bb_tree_map.erase(matrix_bb_tree_map.begin() + larger_position_in_matrix);

		//replace the first element with the newly merged one 
		matrix_bb_tree_map.at(smaller_position_in_matrix) = leaf_nodes.size() + i;
		//replace vertically 
		for (int x = 0; x < smaller_position_in_matrix; x++)
		{
			BoundingBoxHelper::get_combined_bounding_box(&temp_bb, &axis_aligned_bb_tree_[matrix_bb_tree_map.at(x)].bb,
			                                             &axis_aligned_bb_tree_[matrix_bb_tree_map.at(
				                                             smaller_position_in_matrix)].bb);
			float d = BoundingBoxHelper::get_max_length_of_bb(&temp_bb);
			distance_matrix[x][smaller_position_in_matrix] = d;
		}

		//replace horizontally 
		for (int x = smaller_position_in_matrix + 1; x < distance_matrix.size() - 1 - smaller_position_in_matrix; x++)
		{
			BoundingBoxHelper::get_combined_bounding_box(
				&temp_bb, &axis_aligned_bb_tree_[matrix_bb_tree_map.at(smaller_position_in_matrix)].bb,
				&axis_aligned_bb_tree_[matrix_bb_tree_map.at(x)].bb);
			float d = BoundingBoxHelper::get_max_length_of_bb(&temp_bb);
			distance_matrix[smaller_position_in_matrix][x] = d;
		}

		BoundingBoxHelper::get_combined_bounding_box(&temp_bb, &axis_aligned_bb_tree_[matrix_bb_tree_map.at(0)].bb,
		                                             &axis_aligned_bb_tree_[matrix_bb_tree_map.at(1)].bb);
		smallest_box = BoundingBoxHelper::get_max_length_of_bb(&temp_bb);
		next_merge = {0, 1};

		//get the smallest distance
		for (unsigned int x = 0; x < distance_matrix.size(); x++)
		{
			for (unsigned int y = x; y < distance_matrix.size(); y++)
			{
				float d = distance_matrix[x][y];
				if (d < smallest_box && x != y)
				{
					smallest_box = d;
					next_merge = {x, y};
				}
			}
		}
	}
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::cout << "stacked bb build time: " << elapsed_seconds.count() << "s\n";
}


kdTreeElement* StackedBB::get_scene_bb_entry_element() const
{
	if (scene_bb_entry_id_ == -1) return nullptr;
	return &axis_aligned_bb_tree_[scene_bb_entry_id_];
}

kdTreeElement* StackedBB::get_scene_bb_element(unsigned int id) const
{
	return &axis_aligned_bb_tree_[id];
}

collider_modifier* StackedBB::get_scene_bb_element_leaf(const kdTreeElement* leaf_node) const
{
	if (!is_bb_element_leaf_node(leaf_node))
	{
		std::cerr << "not a leaf node\n";
		return nullptr;
	}

	if (leaf_node->child_1 > leaf_nodes.size())
	{
		std::cerr << "shouldnt happen\n";
		return nullptr;
	}
	return leaf_nodes.at(leaf_node->child_1);
}

bool StackedBB::is_bb_element_leaf_node(const kdTreeElement* leaf_node)
{
	return leaf_node->child_0 == -1;
}

void StackedBB::scene_geometry_proximity_check(const glm::vec3& proximity_center,
                                               float radius, ray_cast_result* result)
{
	result->hit = false;
	result->distance_from_origin = std::numeric_limits<float>::max();
	//if scene provides bb tree search it
	if (get_scene_bb_entry_element() != nullptr)
	{
		auto bb_root_node = get_scene_bb_entry_element();
		recurse_proximity_check_bb_tree(bb_root_node, proximity_center, radius, result);
		return;
	}
	std::cerr << "no entry object in stacked BB\n";
}

void StackedBB::scene_geometry_raycast(const glm::vec3& ray_start, const glm::vec3& ray_direction,
                                       ray_cast_result* result, float ray_length, Object3D* ignore, bool ignore_back_face)
{
	auto start = std::chrono::system_clock::now();

	result->hit = false;
	result->distance_from_origin = std::numeric_limits<float>::max();

	
	//if scene provides bb tree search it
	if (get_scene_bb_entry_element() != nullptr)
	{
		auto bb_root_node = get_scene_bb_entry_element();
		recursive_scene_geometry_raycast(bb_root_node, ray_start, ray_direction, result, ray_length, ignore, ignore_back_face);
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		//auto time_string = "ray cast in " + std::to_string(elapsed_seconds.count()) + "seconds\n";
		//scene_->get_global_context()->logger->print_info(time_string);

		return;
	}
	
	std::cerr << "no entry object in stacked BB\n";
}

void StackedBB::recurse_proximity_check_bb_tree(const kdTreeElement* bb_to_check,
                                                const glm::vec3& proximity_center,
                                                float radius, ray_cast_result* result)
{
	if (!BoundingBoxHelper::is_in_bounding_box(&bb_to_check->bb, proximity_center, radius))
	{
		result->hit = false;
		return;
	}

	//is leaf node
	if (is_bb_element_leaf_node(bb_to_check))
	{
		auto coll = get_scene_bb_element_leaf(bb_to_check);

		coll->is_in_proximity(proximity_center, radius, result);
		return;
	}

	//not a leaf node -> continue in the closest child
	auto child_0 = get_scene_bb_element(bb_to_check->child_0);
	auto child_1 = get_scene_bb_element(bb_to_check->child_1);
	if (distance(BoundingBoxHelper::get_center_of_bb(&child_0->bb), proximity_center) <
		distance(BoundingBoxHelper::get_center_of_bb(&child_1->bb), proximity_center))
	{
		//child 1 closer
		recurse_proximity_check_bb_tree(child_0, proximity_center, radius, result);
		if (result->hit) return;

		recurse_proximity_check_bb_tree(child_1, proximity_center, radius, result);
		if (result->hit) return;
	}
	else
	{
		recurse_proximity_check_bb_tree(child_1, proximity_center, radius, result);
		if (result->hit) return;

		recurse_proximity_check_bb_tree(child_0, proximity_center, radius, result);
		if (result->hit) return;
	}
}

void StackedBB::recursive_scene_geometry_raycast(const kdTreeElement* bb_to_check, const glm::vec3& ray_start,
                                                 const glm::vec3& ray_direction, ray_cast_result* result, float ray_length, Object3D* ignore, bool ignore_back_face)
{
	if (!BoundingBoxHelper::ray_axis_aligned_bb_intersection(&bb_to_check->bb, ray_start, ray_direction, result) &&
		!BoundingBoxHelper::is_in_bounding_box(&bb_to_check->bb, ray_start))
	{
		result->hit = false;
		return;
	}
	
	//is leaf node
	if (is_bb_element_leaf_node(bb_to_check))
	{
		auto coll = get_scene_bb_element_leaf(bb_to_check);
		if (coll->get_parent() == ignore)
		{
			result->hit = false;
			return;
		}
		
		coll->ray_intersection_world_space(ray_start, ray_direction,ray_length , ignore_back_face, result);
		if (result->hit)
		{
			result->object_3d = coll->get_parent();
		}
		return;
	}

	auto r_0 = ray_cast_result();
	auto r_1 = ray_cast_result();
	
	auto child_0 = get_scene_bb_element(bb_to_check->child_0);
	auto child_1 = get_scene_bb_element(bb_to_check->child_1);

	recursive_scene_geometry_raycast(child_0, ray_start, ray_direction, &r_0, ray_length, ignore, ignore_back_face);
	recursive_scene_geometry_raycast(child_1, ray_start, ray_direction, &r_1, ray_length, ignore, ignore_back_face);

	if (r_0.hit && r_1.hit)
	{
		//hit in both boxes
		if (r_0.distance_from_origin < r_1.distance_from_origin)
		{
			//r0 is closer
			copy_ray_cast_result(&r_0,result);
			return;
		} else
		{
			copy_ray_cast_result(&r_1,result);
			return;
		}
	}
	if (r_0.hit)
	{
		copy_ray_cast_result(&r_0,result);
		return;
	}

	if (r_1.hit)
	{
		copy_ray_cast_result(&r_1,result);
		return;
	}
	result->hit = false;
}
