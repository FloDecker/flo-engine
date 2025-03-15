#include "mesh_collider.h"

#include <random>

#include "../../../../../Util/math_util.h"
#include "../../Content/Mesh.h"
#include "../../Util/BoundingBoxHelper.h"

//TODO : this can be heavily optimized, calculate once in local space and transform with the parent 
StructBoundingBox mesh_collider::calculate_world_space_bounding_box_internal_()
{
	StructBoundingBox bounding_box;
	StructBoundingBox temp_bb;
	temp_bb.max = parent->getGlobalTransform() * glm::vec4(vertex_arrays_.at(0)->vertices->at(0).position, 1);
	temp_bb.min = temp_bb.max;

	bounding_box.max = temp_bb.max;
	bounding_box.min = temp_bb.max;

	for (auto vertex_array : vertex_arrays_)
	{
		BoundingBoxHelper::get_bounding_box_from_vertex_array(
			&temp_bb, vertex_array, parent->getGlobalTransform());
		BoundingBoxHelper::get_combined_bounding_box(
			&bounding_box,
			&bounding_box, &temp_bb
		);
	}
	return bounding_box;
}

mesh_collider::mesh_collider(Object3D* parent_game_object_3d, Mesh* mesh): collider_modifier(parent_game_object_3d)
{
	for (auto vertex_array : mesh->vertexArrays)
	{
		vertex_arrays_.push_back(vertex_array->get_vertex_array());
	}
}

void mesh_collider::ray_intersection_local_space(glm::vec3 ray_origin_ls, glm::vec3 ray_direction_ls, float ray_length,
                                                 bool ignore_back_face, ray_cast_result* ray_cast_result_out)
{
	*ray_cast_result_out = {};
	std::vector<struct_vertex_array*> vertex_arrays_of_geometry = vertex_arrays_;
	for (unsigned int a = 0; a < vertex_arrays_of_geometry.size(); a++)
	{
		struct_vertex_array* vertex_array = vertex_arrays_of_geometry.at(a);
		for (unsigned int i = 0; i < vertex_array->indices->size(); i += 3)
		{
			vertex v0 = vertex_array->vertices->at(vertex_array->indices->at(i));
			vertex v1 = vertex_array->vertices->at(vertex_array->indices->at(i + 1));
			vertex v2 = vertex_array->vertices->at(vertex_array->indices->at(i + 2));


			glm::vec3 face_normal = normalize(v0.normal + v1.normal + v2.normal);

			if (ignore_back_face && dot(face_normal, ray_direction_ls) > 0) continue;

			float d = -dot(face_normal, v0.position);
			float t =
				-((dot(face_normal, ray_origin_ls) + d) /
					dot(face_normal, ray_direction_ls));

			glm::vec3 hit_point = ray_origin_ls + t * ray_direction_ls;
			auto c = glm::vec3();

			//if (!is_point_in_triangle(v0.position,v1.position,v2.position,hit_point)) continue;


			// Edge 0
			glm::vec3 edge0 = v1.position - v0.position;
			glm::vec3 vp0 = hit_point - v0.position;
			c = cross(edge0, vp0);
			if (dot(face_normal, c) < 0) continue; // P is on the right side

			// Edge 1
			glm::vec3 edge1 = v2.position - v1.position;
			glm::vec3 vp1 = hit_point - v1.position;
			c = cross(edge1, vp1);
			if (dot(face_normal, c) < 0) continue; // P is on the right side

			// Edge 2
			glm::vec3 edge2 = v0.position - v2.position;
			glm::vec3 vp2 = hit_point - v2.position;
			c = cross(edge2, vp2);
			if (dot(face_normal, c) < 0) continue; // P is on the right side

			auto hit_ws = glm::vec3(parent->getGlobalTransform() * glm::vec4(hit_point, 1));
			auto distance = glm::distance(hit_ws, ray_origin_ls);
			// This ray hits the triangle
			if (ray_cast_result_out->distance_from_origin > distance)
			{
				ray_cast_result_out->distance_from_origin = distance;
				ray_cast_result_out->hit = true;
				ray_cast_result_out->hit_local = hit_point;
				ray_cast_result_out->hit_normal_local = face_normal;
				ray_cast_result_out->hit_world_space = hit_ws;
				ray_cast_result_out->hit_normal_world_space = parent->getGlobalTransform() * glm::vec4(
					ray_cast_result_out->hit_normal_local, 0);
				ray_cast_result_out->object_3d = parent;
				ray_cast_result_out->vertex_indices[0] = vertex_array->indices->at(i);
				ray_cast_result_out->vertex_indices[1] = vertex_array->indices->at(i + 1);
				ray_cast_result_out->vertex_indices[2] = vertex_array->indices->at(i + 2);
			}
		}
	}
}

struct_intersection mesh_collider::check_intersection(collider_modifier* other)
{
	return other->check_intersection_with(this);
}

struct_intersection mesh_collider::check_intersection_with(box_collider* box)
{
	std::cerr << "function not implemented mesh_collider::check_intersection_with\n";
	return struct_intersection();
}

void mesh_collider::is_in_proximity(glm::vec3 center_ws, float radius, ray_cast_result* result)
{
	ray_cast_result temp_ray_cast_result;

	const std::vector<struct_vertex_array*>* vertex_arrays_of_geometry = &vertex_arrays_;
	for (unsigned int a = 0; a < vertex_arrays_of_geometry->size(); a++)
	{
		is_in_proximity_vertex_array(center_ws, radius, a, &temp_ray_cast_result);
		if (temp_ray_cast_result.hit && temp_ray_cast_result.distance_from_origin < result->distance_from_origin)
		{
			std::memcpy(result, &temp_ray_cast_result, sizeof(ray_cast_result));
		}
	}
}

void mesh_collider::is_in_proximity_vertex_array(glm::vec3 center_ws, float radius, unsigned int vertex_array_id,
                                                 ray_cast_result* result) const
{
	result->hit = false;
	result->distance_from_origin = std::numeric_limits<float>::max();

	ray_cast_result temp_result;
	glm::vec3 proximity_center_local = get_parent()->transform_position_to_local_space(center_ws);
	std::vector<struct_vertex_array*> vertex_arrays_of_geometry = this->vertex_arrays_;
	struct_vertex_array* vertex_array = vertex_arrays_of_geometry.at(vertex_array_id);
	for (unsigned int i = 0; i < vertex_array->indices->size(); i += 3)
	{
		vertex v0 = vertex_array->vertices->at(vertex_array->indices->at(i));
		vertex v1 = vertex_array->vertices->at(vertex_array->indices->at(i + 1));
		vertex v2 = vertex_array->vertices->at(vertex_array->indices->at(i + 2));
		glm::vec3 face_normal = normalize(v0.normal + v1.normal + v2.normal);

		RayIntersectionHelper::get_closest_point_on_triangle(v0.position, v1.position, v2.position,
		                                                     proximity_center_local, &temp_result);

		temp_result.hit_world_space = get_parent()->transform_vertex_to_world_space(temp_result.hit_local);
		temp_result.distance_from_origin = distance(temp_result.hit_world_space, center_ws);

		if (temp_result.distance_from_origin < radius && temp_result.distance_from_origin < result->
			distance_from_origin)
		{
			result->hit = true;
			result->distance_from_origin = temp_result.distance_from_origin;
			result->hit_local = temp_result.hit_local;
			result->hit_world_space = temp_result.hit_world_space;
			result->hit_normal_local = temp_result.hit_normal_local;
			result->hit_normal_world_space = get_parent()->
				transform_vector_to_world_space(temp_result.hit_normal_local);
		}
	}
}

struct_intersection mesh_collider::check_intersection_with(mesh_collider* mesh)
{
	std::cerr << "function not implemented mesh_collider::check_intersection_with\n";
	return struct_intersection();
}

glm::vec3 mesh_collider::get_center_of_mass_local()
{
	auto center_of_mass = glm::vec3(0, 0, 0);
	int vertices = 0;
	for (unsigned int i = 0; i < vertex_arrays_.size(); i++)
	{
		auto vertex_array = this->vertex_arrays_.at(i);
		vertices += vertex_array->vertices->size();
		for (unsigned int j = 0; j < vertex_array->vertices->size(); j++)
		{
			vertex v = vertex_array->vertices->at(j);
			center_of_mass += v.position;
		}
	}
	return center_of_mass / static_cast<float>(vertices);
}

void mesh_collider::scatter_points_on_surface(std::vector<vertex>* points, unsigned amount)
{
	std::cout << "scatter_points_on_surface\n";
	std::random_device rd; // Seed the random number generator
	std::mt19937 gen(rd()); // Mersenne Twister PRNG
	std::uniform_int_distribution<int> dist_vertex_arrays(0, vertex_arrays_.size() - 1); // Range [1, 100]

	int randomNumber = dist_vertex_arrays(gen);
	std::cout << "scatter_points_on_surface 2\n";

	for (int i = 0; i < amount; i++)
	{
		auto v = vertex_arrays_.at(dist_vertex_arrays(gen));
		std::uniform_int_distribution<int> dist_vertices(0, v->indices->size() - 1); // Range [1, 100]
		auto random_index = dist_vertices(gen);
		random_index -= random_index % 3;
		random_index = std::max(0, random_index);
		auto v_1 = v->vertices->at(v->indices->at(random_index));
		auto v_2 = v->vertices->at(v->indices->at(random_index + 1));
		auto v_3 = v->vertices->at(v->indices->at(random_index + 2));

		auto p = math_util::get_random_point_in_triangle(v_1.position, v_2.position, v_3.position);
		auto n = normalize(v_1.normal + v_2.normal + v_3.normal);
		points->push_back({parent->transform_vertex_to_world_space(p), parent->transform_vector_to_world_space(n)});
	}
}
