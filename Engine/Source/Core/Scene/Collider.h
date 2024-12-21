#pragma once
#include "Mesh3D.h"
#include "Object3D.h"
#include "../../Util/BoundingBoxHelper.h"
#include "../../Util/RayIntersectionHelper.h"
#include "SceneTools/VoxelizerTools/AbstractVoxelizer.h"
#include "SceneTools/VoxelizerTools/AbstractVoxelizer.h"
#include "SceneTools/VoxelizerTools/AbstractVoxelizer.h"
#include "SceneTools/VoxelizerTools/AbstractVoxelizer.h"

class Collider : public Object3D
{
public:
	Collider(Object3D* parent);
	void check_collision_ws(glm::vec3 ray_origin_ws, glm::vec3 ray_direction_ws, float ray_length,
	                        bool ignore_back_face, ray_cast_hit* ray_cast_hit);
	virtual void check_collision_ls(glm::vec3 ray_origin_ls, glm::vec3 ray_direction_ls, float ray_length,
	                                bool ignore_back_face, ray_cast_hit* ray_cast_hit);
	virtual bool is_in_proximity(glm::vec3 center_ws, float radius);
	virtual void calculate_world_space_bounding_box();
	virtual int get_collider_type();
	virtual void visualize_collider(); //for debugging
	int drawSelf() override;

	glm::vec3 get_center_of_mass();
	glm::mat3 get_inertia_tensor();
	StructBoundingBox bounding_box;

protected:
	//center of mass
	bool center_of_mass_calculated_internal_ = false;
	glm::vec3 center_of_mass_internal_;
	virtual glm::vec3 calculate_center_of_mass_internal();

	//inertial tensor
	bool inertia_tensor_calculated_internal_ = false;
	glm::mat3 inertia_tensor_internal_;
	virtual glm::mat3 calculate_inertia_tensor_internal();
};

class MeshCollider : public Collider
{
public:
	MeshCollider(Object3D* parent, const std::vector<struct_vertex_array*>& vertex_arrays);
	MeshCollider(Object3D* parent, Mesh3D* mesh);
	std::vector<struct_vertex_array*>* get_vertex_arrays();
	void check_collision_ls(glm::vec3 ray_origin_ls, glm::vec3 ray_direction_ls, float ray_length,
	                        bool ignore_back_face,
	                        ray_cast_hit* ray_cast_hit) override;
	void visualize_collider() override;
	bool is_in_proximity(glm::vec3 center_ws, float radius) override;
	int get_collider_type() override;
	bool is_in_proximity_vertex_array(glm::vec3 center_ws, float radius, unsigned int vertex_array_id);
	bool is_in_proximity_vertex(float radius, unsigned int v_0, glm::vec3 proximity_center_local,
	                            struct_vertex_array* vertex_array) const;
	void calculate_world_space_bounding_box() override;

protected:
	glm::vec3 calculate_center_of_mass_internal() override;
	glm::mat3 calculate_inertia_tensor_internal() override;

private:
	std::vector<struct_vertex_array*> vertex_arrays_;
};
