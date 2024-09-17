#include "Voxelizer.h"

#include <numeric>
#include <set>

#include "AbstractVoxelizer.h"


#define FLOATING_POINT_ACCEPTANCE 0.02

Voxelizer::Voxelizer(GlobalContext* global_context, SceneContext* scene_context): AbstractVoxelizer(
	global_context, scene_context)
{
};

glm::i16vec3 Voxelizer::cubic_expansion_directions[26] = {
	glm::i16vec3(1, 1, 1),
	glm::i16vec3(1, 0, 1),
	glm::i16vec3(1, -1, 1),
	glm::i16vec3(0, 1, 1),
	glm::i16vec3(0, 0, 1),
	glm::i16vec3(0, -1, 1),
	glm::i16vec3(-1, 1, 1),
	glm::i16vec3(-1, 0, 1),
	glm::i16vec3(-1, -1, 1),

	glm::i16vec3(1, 1, 0),
	glm::i16vec3(1, 0, 0),
	glm::i16vec3(1, -1, 0),
	glm::i16vec3(0, 1, 0),
	glm::i16vec3(0, -1, 0),
	glm::i16vec3(-1, 1, 0),
	glm::i16vec3(-1, 0, 0),
	glm::i16vec3(-1, -1, 0),

	glm::i16vec3(1, 1, -1),
	glm::i16vec3(1, 0, -1),
	glm::i16vec3(1, -1, -1),
	glm::i16vec3(0, 1, -1),
	glm::i16vec3(0, 0, -1),
	glm::i16vec3(0, -1, -1),
	glm::i16vec3(-1, 1, -1),
	glm::i16vec3(-1, 0, -1),
	glm::i16vec3(-1, -1, -1),
};

void Voxelizer::recalculate()
{
	auto start = std::chrono::system_clock::now();

	auto transform_global = getGlobalTransform();
	glm::vec3 upper_right_corner = glm::round(transform_global * glm::vec4(1, 1, 1, 1));
	glm::vec3 lower_left_corner = glm::round(transform_global * glm::vec4(-1, -1, -1, 1));

	unsigned int distance_x = static_cast<int>(abs(upper_right_corner.x - lower_left_corner.x)) * voxel_precision;
	unsigned int distance_y = static_cast<int>(abs(upper_right_corner.y - lower_left_corner.y)) * voxel_precision;
	unsigned int distance_z = static_cast<int>(abs(upper_right_corner.z - lower_left_corner.z)) * voxel_precision;


	if (upper_right_corner.x - lower_left_corner.x < 1.0)
	{
		std::cout << "needs an extension in x of at least 1\n";
	}
	if (upper_right_corner.y - lower_left_corner.y < 1.0)
	{
		std::cout << "needs an extension in y of at least 1\n";
	}
	if (upper_right_corner.z - lower_left_corner.z < 1.0)
	{
		std::cout << "needs an extension in z of at least 1\n";
	}

	calculate_area_filled_by_polygons(scene_context_);

	//calculate_area_filled_recursive(scene_context_, upper_right_corner, lower_left_corner,
	//                                {distance_x, distance_y, distance_z}, {0, 0, 0});

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::cout << "voxel build time: " << elapsed_seconds.count() << "s\n";
	std::cout << "build " << voxel_positions_.size() << " voxels\n";
}

//load the zero level voxels into 3d texture where every surface point is 1 and the empty space is 0
//(just use the distance function it's way faster to traverse)
void Voxelizer::load_into_voxel_texture(Texture3D* texture_3d)
{
	auto transform_global_inverse = glm::inverse(getGlobalTransform());


	for (auto v : voxel_positions_)
	{
		glm::vec3 object_space = transform_global_inverse * glm::vec4(v, 1.0);
		object_space *= 0.5f;
		object_space += glm::vec3(0.5);
		texture_3d->write_to_voxel_field_float(15, 15, 15, 15, object_space.x, object_space.y, object_space.z);
	}
}

//load the zero level voxels into a 3d texture where every surface point is 0 and the empty space is the distance to the zero level set
void Voxelizer::load_into_voxel_texture_df(Texture3D* texture_3d)
{
	int current_level = 0;
	std::vector<glm::i16vec3> current_set = zero_level_set;

	if (is_level_set_matrix_initialized())
	{
		reset_level_set_matrix();
	}
	else
	{
		create_level_set_matrix();
	}
	std::vector<glm::i16vec3> next_set;
	std::cout << "Building level set for distance function :";
	while (!current_set.empty())
	{
		next_set.clear();
		std::cout << current_level << " | ";

		for (auto position : current_set)
		{
			for (auto offset : cubic_expansion_directions)
			{
				glm::i16vec3 test_pos = position + offset;
				if (is_in_level_set_matrix(test_pos))
				{
					//if inside boundaries
					int level_at_pos = get_value_of_level_set_matrix_at(test_pos);
					if (level_at_pos == -1)
					{
						insert_into_level_set_matrix(test_pos, current_level + 1);
						next_set.push_back(test_pos); // insert value into set for next interation;
						//TODO: change datatype of 3d texture to fit more distances 
						texture_3d->write_to_voxel_field(0, 0, 0, (current_level + 1 < 14) ? current_level + 1 : 14,
						                                 test_pos.x, test_pos.y, test_pos.z);


						//just for debugging
						/*
						if (current_level == 3)
						{
						    auto transform_global = getGlobalTransform();
						    glm::vec3 ws_lower_left = glm::round(transform_global * glm::vec4(-1, -1, -1, 1));
						
						    float step_size = 1.0f / static_cast<float>(voxel_precision);
						    auto ws_pos = ws_lower_left + static_cast<glm::vec3>(test_pos) * step_size + step_size * 0.5f;
						
						    auto c = new Cube3D(global_context_);

						    
						    this->addChild(c);
						    c->setScale(glm::vec3(0.01,0.01,0.01));
						    float interpol;
						    interpol = static_cast<float>(current_level) / 4.0f;

						    c->color = glm::vec3(1, 0, 0)*interpol+glm::vec3(0, 1, 0)*(1.0f-interpol);
						    c->set_position_global(ws_pos);
						}
*/
					}
				}
			}
		}

		current_level += 1; //increment current level set
		current_set = next_set; //copy next set positions to new set;
	}
	std::cout << "\n";
}

bool Voxelizer::create_level_set_matrix()
{
	if (is_level_set_matrix_initialized())
	{
		std::wcerr << "level set matrix has already been initialized";
		return false;
	}
	auto transform_global = getGlobalTransform();
	glm::vec3 upper_right_corner = glm::round(transform_global * glm::vec4(1, 1, 1, 1));
	glm::vec3 lower_left_corner = glm::round(transform_global * glm::vec4(-1, -1, -1, 1));
	unsigned int distance_x = static_cast<int>(abs(upper_right_corner.x - lower_left_corner.x)) * voxel_precision;
	unsigned int distance_y = static_cast<int>(abs(upper_right_corner.y - lower_left_corner.y)) * voxel_precision;
	unsigned int distance_z = static_cast<int>(abs(upper_right_corner.z - lower_left_corner.z)) * voxel_precision;

	level_set_matrix_dimensions = glm::i16vec3(distance_x, distance_y, distance_z);

	int level_set_size = distance_x * distance_y * distance_z;

	level_set_matrix = new int[level_set_size](); //init all entrys as -1

	if (!level_set_matrix)
	{
		std::cerr << "steps from zero array matrix memory couldn't be allocated";
		return false;
	}

	is_level_set_matrix_initialized_ = true;
	memset(level_set_matrix, -1, sizeof(int) * level_set_size);
	return true;
}

bool Voxelizer::reset_level_set_matrix()
{
	if (is_level_set_matrix_initialized())
	{
		memset(level_set_matrix, -1, sizeof(int) * get_length_of_level_set_matrix());
		return true;
	}
	std::cerr << "cant reset level set matrix since it hasn't been initialized\n";
	return false;
}

int Voxelizer::get_length_of_level_set_matrix() const
{
	if (is_level_set_matrix_initialized())
	{
		const auto a = get_dimensions_of_level_set_matrix();
		return a.x * a.y * a.z;
	}
	std::cerr << "cant get length since level set matrix hasn't been initialized\n";
	return -1;
}

bool Voxelizer::is_level_set_matrix_initialized() const
{
	return is_level_set_matrix_initialized_;
}

void Voxelizer::insert_into_level_set_matrix(glm::i16vec3 pos, int i) const
{
	if (!is_level_set_matrix_initialized())
	{
		std::cerr << "level set matrix hasn't been initialized\n";
	}
	if (!is_in_level_set_matrix(pos))
	{
		std::cerr << "point isn't in level set matrix\n";
	}

	level_set_matrix[level_set_matrix_dimensions.x * level_set_matrix_dimensions.y * pos.z + level_set_matrix_dimensions
		.x * pos.y +
		pos.x] = i;
}

int Voxelizer::get_value_of_level_set_matrix_at(glm::i16vec3 pos) const
{
	if (!is_level_set_matrix_initialized())
	{
		std::cerr << "level set matrix hasn't been initialized\n";
		return -1;
	}
	if (!is_in_level_set_matrix(pos))
	{
		std::cerr << "point isn't in level set matrix\n";
		return -1;
	}
	return level_set_matrix[level_set_matrix_dimensions.x * level_set_matrix_dimensions.y * pos.z +
		level_set_matrix_dimensions.x * pos.y +
		pos.x];
}

bool Voxelizer::is_in_level_set_matrix(glm::i16vec3 pos) const
{
	if (!is_level_set_matrix_initialized())
	{
		return false;
	}
	return (pos.x >= 0 && pos.y >= 0 && pos.z >= 0
		&& pos.x < level_set_matrix_dimensions.x && pos.y < level_set_matrix_dimensions.y && pos.z <
		level_set_matrix_dimensions.z);
}

glm::i16vec3 Voxelizer::get_dimensions_of_level_set_matrix() const
{
	if (is_level_set_matrix_initialized())
	{
		return level_set_matrix_dimensions;
	}

	std::cerr << "cant get dimensions since level set matrix hasn't been initialized\n";
	return {-1, -1, -1,};
}

glm::vec3 Voxelizer::get_ws_pos_from_voxel_pos(glm::i16vec3 pos) const
{
	glm::vec3 pos_object_space = static_cast<glm::vec3>(pos) / static_cast<glm::vec3>(
		get_dimensions_of_level_set_matrix());
	pos_object_space = (pos_object_space - glm::vec3(0.5)) * glm::vec3(2.0);
	return transform_vertex_to_world_space(pos_object_space);
}

glm::i16vec3 Voxelizer::get_level_set_matrix_pos_from_ws(const glm::vec3& ws_pos)
{
	auto transform_global_inverse = glm::inverse(getGlobalTransform()); //TODO: replace with inverse from object 3d
	glm::vec3 object_space = transform_global_inverse * glm::vec4(ws_pos, 1.0);
	object_space *= 0.5f;
	object_space += glm::vec3(0.5);


	auto dimensions = get_dimensions_of_level_set_matrix();
	return {
		static_cast<int>(static_cast<float>(dimensions.x) * object_space.x),
		static_cast<int>(static_cast<float>(dimensions.y) * object_space.y),
		static_cast<int>(static_cast<float>(dimensions.z) * object_space.z)
	};
}

int Voxelizer::get_unique_pos_id_in_matrix(glm::i16vec3 pos) const
{
	return level_set_matrix_dimensions.x * level_set_matrix_dimensions.y * pos.z +
		level_set_matrix_dimensions.x * pos.y +
		pos.x;
}


void Voxelizer::expand_polygon_to_zero_level_set(MeshCollider *collider, unsigned int vertex_id_0, struct_vertex_array* vertex_array, glm::i16vec3 voxel_to_expand_from, float radius)
{
	std::vector<glm::i16vec3> positions_to_check = {voxel_to_expand_from};

	glm::vec3 voxel_pos_ws = get_ws_pos_from_voxel_pos(voxel_to_expand_from);
	auto global_inverse = glm::inverse(collider->getGlobalTransform());
	glm::vec3 proximity_center_local = global_inverse * glm::vec4(voxel_pos_ws, 1);
	if (!collider->is_in_proximity_vertex(radius,vertex_id_0,proximity_center_local,vertex_array))
		return;
	insert_into_level_set_matrix(voxel_to_expand_from, 0);

	std::set<int> set_x ={get_unique_pos_id_in_matrix(voxel_to_expand_from)};
	
	while (!positions_to_check.empty())
	{
		glm::i16vec3 current = positions_to_check.back();
		positions_to_check.pop_back();
		glm::vec3 current_ws = get_ws_pos_from_voxel_pos(current);
		zero_level_set.emplace_back(current);
		for (auto direction : Voxelizer::cubic_expansion_directions)
		{
			auto p = current + direction;
			if (is_in_level_set_matrix(p))
			{
				
				proximity_center_local = global_inverse * glm::vec4(get_ws_pos_from_voxel_pos(p), 1);
				if (set_x.find(get_unique_pos_id_in_matrix(p)) == set_x.end() && collider->is_in_proximity_vertex(radius, vertex_id_0, proximity_center_local,
					                                           vertex_array))
				{
					set_x.insert(get_unique_pos_id_in_matrix(p));

					positions_to_check.emplace_back(p);
					insert_into_level_set_matrix(p, 0);
					//deBUG
					//auto c = new Cube3D(global_context_);
					//this->addChild(c);
					//c->setScale(glm::vec3(0.01));
					//c->color = {0, 1, 0};
					//c->set_position_global(get_ws_pos_from_voxel_pos(p));
				}
			}
		}
	}

	//zero_level_set.push_back(voxel_to_expand_from);
}

void Voxelizer::calculate_area_filled_by_polygons(SceneContext* scene_context)
{
	create_level_set_matrix();
	const auto b = get_as_bounding_box();
	std::vector<std::tuple<MeshCollider*, std::vector<vertex_array_filter>*>>* filtered_colliders = scene_context->
		get_triangles_in_bounding_box(b);
	
	for(std::tuple<MeshCollider*, std::vector<vertex_array_filter>*> filtered_collider : *filtered_colliders)
	{
		
		MeshCollider* collider= std::get<MeshCollider*>(filtered_collider);
		std::vector<vertex_array_filter>* vertex_array_filters = std::get<std::vector<vertex_array_filter>*>(filtered_collider);
		for (vertex_array_filter vertex_array: *vertex_array_filters)
		{
			int vertex_array_id = vertex_array.vertex_array_id;
			for (unsigned int vertex_id : vertex_array.indices)
			{
				
				int contained_id = -1;
				glm::vec3 contained_pos;
				for (int i = 0; i<3;i++)
				{
					auto v = collider->get_vertex_arrays()->at(vertex_array_id);
					contained_pos = collider->transform_vertex_to_world_space(v->vertices->at(v->indices->at(vertex_id+i)).position);
					if(BoundingBoxHelper::is_in_bounding_box(b,contained_pos))
					{
						contained_id = i;
						break;
					}
				}
				if (contained_id != -1)
				{
					std::cout << "|";
					glm::i16vec3 voxel_pos_of_vertex = get_level_set_matrix_pos_from_ws(contained_pos);
					expand_polygon_to_zero_level_set(collider, vertex_id, collider->get_vertex_arrays()->at(vertex_array_id),
													 voxel_pos_of_vertex, 1.0 / 8.0);
				}		
			}
		}
	}
	free(b);


	//get all objects inside the field
}

void Voxelizer::calculate_area_filled_recursive(SceneContext* scene_context, glm::vec3 ws_upper_right,
                                                glm::vec3 ws_lower_left, glm::i16vec3 voxel_upper_right,
                                                glm::i16vec3 voxel_lower_left)
{
	float step_size = 1.0f / static_cast<float>(voxel_precision);

	int distance_x = abs(voxel_upper_right.x - voxel_lower_left.x);
	int distance_y = abs(voxel_upper_right.y - voxel_lower_left.y);
	int distance_z = abs(voxel_upper_right.z - voxel_lower_left.z);

	//if all distances are the same we can do a distance check from the center
	if (distance_x == distance_y && distance_x == distance_z)
	{
		glm::vec3 center = ws_lower_left + (static_cast<glm::vec3>(voxel_lower_left) + static_cast<glm::vec3>(
				voxel_upper_right)) *
			step_size * 0.5f;

		//float radius = glm::length(glm::vec3(step_size)) * 0.5f;
		float radius = glm::length(glm::vec3(step_size)) * distance_x * 0.5;

		if (scene_context_->get_bb()->scene_geometry_proximity_check(center, radius))
		{
			//raycast hit and smallest step size reached
			if (distance_x == 1 && distance_y == 1 && distance_z == 1)
			{
				auto ws_pos = ws_lower_left + static_cast<glm::vec3>(voxel_lower_left) * step_size + step_size * 0.5f;

				//auto c = new Cube3D(global_context_);
				//this->addChild(c);
				//c->setScale(distance_x * (step_size / this->get_scale().x), distance_x * (step_size / this->get_scale().y),
				//            distance_x * (step_size / this->get_scale().z));
				//c->color = {0, 1, 0};
				//c->set_position_global(ws_pos);

				voxel_positions_.push_back(ws_pos);
				zero_level_set.push_back(voxel_lower_left);
				return;
			}
			else
			{
				//auto c = new Cube3D(global_context_);
				//this->addChild(c);
				//c->setScale(distance_x * (step_size / this->get_scale().x), distance_x * (step_size / this->get_scale().y),
				//            distance_x * (step_size / this->get_scale().z));
				//c->color = {1, 0, 0};
				//c->set_position_global(center);

				int distance = (distance_x + 1) / 2;
				glm::i16vec3 offset_of_chunk = glm::i16vec3(distance);
				for (int x = 0; x <= 1; x++)
				{
					for (int y = 0; y <= 1; y++)
					{
						for (int z = 0; z <= 1; z++)
						{
							glm::i16vec3 offset = {x * distance, y * distance, z * distance};
							calculate_area_filled_recursive(scene_context, ws_upper_right,
							                                ws_lower_left, voxel_lower_left + offset + offset_of_chunk,
							                                voxel_lower_left + offset);
						}
					}
				}
			}
			//countinue seperating
		}
		else
		{
			// no hit in chunk, abort
			//TEMP FOR TEST
			return;
		}
	}


	if (distance_x <= distance_y && distance_x <= distance_z) // x is the smallest
	{
	}
	if (distance_y <= distance_x && distance_y <= distance_z) // y is the smallest
	{
	}
	if (distance_z <= distance_x && distance_z <= distance_y) // z is the smallest
	{
	}
}
