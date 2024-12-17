#include "Collider.h"

#include <gtx/string_cast.hpp>

#include "SceneTools/VoxelizerTools/AbstractVoxelizer.h"
#include "SceneTools/VoxelizerTools/AbstractVoxelizer.h"
#include "SceneTools/VoxelizerTools/AbstractVoxelizer.h"
#include "SceneTools/VoxelizerTools/AbstractVoxelizer.h"
#include "SceneTools/VoxelizerTools/AbstractVoxelizer.h"
#include "SceneTools/VoxelizerTools/AbstractVoxelizer.h"
#include "SceneTools/VoxelizerTools/AbstractVoxelizer.h"
#include "SceneTools/VoxelizerTools/AbstractVoxelizer.h"


Collider::Collider(Object3D *parent): Object3D(parent)
{
    
}

void Collider::check_collision_ws(glm::vec3 ray_origin_ws, glm::vec3 ray_direction_ws, float ray_length,
    bool ignore_back_face, RayCastHit* ray_cast_hit)
{
    auto global_inverse = glm::inverse(this->transformGlobal);
    glm::vec4 ray_cast_origin_vec4_local = global_inverse * glm::vec4(ray_origin_ws, 1);
    glm::vec3 ray_cast_origin_vec3_local = glm::vec3(ray_cast_origin_vec4_local);

    glm::vec4 ray_cast_direction_vec4_local = global_inverse * glm::vec4(ray_direction_ws, 0);
    glm::vec3 ray_cast_direction_vec3_local = glm::vec3(ray_cast_direction_vec4_local);
    check_collision_ls(ray_cast_origin_vec3_local, ray_cast_direction_vec3_local, ray_length, ignore_back_face, ray_cast_hit);
}

void Collider::check_collision_ls(glm::vec3 ray_origin_ls, glm::vec3 ray_direction_ls, float ray_length,
                                  bool ignore_back_face,
                                  RayCastHit* ray_cast_hit)
{
}

bool Collider::is_in_proximity(glm::vec3 center_ws, float radius)
{
    return false;
}

void Collider::calculate_world_space_bounding_box()
{
}

int Collider::get_collider_type()
{
    return -1;
}

glm::vec3 Collider::get_center_of_mass()
{
    if (!center_of_mass_calculated_internal_)
    {
        center_of_mass_internal_ = calculate_center_of_mass_internal();
        center_of_mass_calculated_internal_ = true;
    }
    return center_of_mass_internal_;
}

glm::mat3 Collider::get_inertia_tensor()
{
    if (!inertia_tensor_calculated_internal_)
    {
        inertia_tensor_internal_ = calculate_inertia_tensor_internal();
        inertia_tensor_calculated_internal_ = true;
    }
    return inertia_tensor_internal_;
}

glm::vec3 Collider::calculate_center_of_mass_internal()
{
    return glm::vec3();
}


void Collider::visualize_collider()
{
}

int Collider::drawSelf()
{
    if (renderContext->flags.visualize_hitboxes)
    {
        visualize_collider();
    }
    return 1;
}


glm::mat3 Collider::calculate_inertia_tensor_internal()
{
    return glm::mat3();
}

//Mesh Collider
MeshCollider::MeshCollider(Object3D *parent,
                           const std::vector<struct_vertex_array*>& vertex_arrays): Collider(parent)
{
    vertex_arrays_ = vertex_arrays;
}

MeshCollider::MeshCollider(Object3D *parent, Mesh3D* mesh): Collider(parent)
{
    for (auto vertex_array : mesh->get_mesh()->vertexArrays)
    {
        vertex_arrays_.push_back(vertex_array->get_vertex_array());
    }
    name = "collider of " + mesh->name;
}

std::vector<struct_vertex_array*>* MeshCollider::get_vertex_arrays()
{
    return &vertex_arrays_;
}

void MeshCollider::check_collision_ls(glm::vec3 ray_origin_ls, glm::vec3 ray_direction_ls, float ray_length,
                                      bool ignore_back_face,
                                      RayCastHit* ray_cast_hit)
{
    std::vector<struct_vertex_array*>* vertex_arrays_of_geometry = this->get_vertex_arrays();
    for (unsigned int a = 0; a < vertex_arrays_of_geometry->size(); a++)
    {
        struct_vertex_array* vertex_array = vertex_arrays_of_geometry->at(a);
        for (unsigned int i = 0; i < vertex_array->indices->size(); i += 3)
        {
            vertex v0 = vertex_array->vertices->at(vertex_array->indices->at(i));
            vertex v1 = vertex_array->vertices->at(vertex_array->indices->at(i + 1));
            vertex v2 = vertex_array->vertices->at(vertex_array->indices->at(i + 2));


            glm::vec3 face_normal = glm::normalize(v0.normal + v1.normal + v2.normal);

            if (ignore_back_face && glm::dot(face_normal, ray_direction_ls) > 0) continue;

            float d = -glm::dot(face_normal, v0.position);
            float t =
                -((glm::dot(face_normal, ray_origin_ls) + d) /
                    glm::dot(face_normal, ray_direction_ls));

            glm::vec3 hit_point = ray_origin_ls + t * ray_direction_ls;
            auto c = glm::vec3();

            //if (!is_point_in_triangle(v0.position,v1.position,v2.position,hit_point)) continue;


            // Edge 0
            glm::vec3 edge0 = v1.position - v0.position;
            glm::vec3 vp0 = hit_point - v0.position;
            c = glm::cross(edge0, vp0);
            if (glm::dot(face_normal, c) < 0) continue; // P is on the right side

            // Edge 1
            glm::vec3 edge1 = v2.position - v1.position;
            glm::vec3 vp1 = hit_point - v1.position;
            c = glm::cross(edge1, vp1);
            if (glm::dot(face_normal, c) < 0) continue; // P is on the right side

            // Edge 2
            glm::vec3 edge2 = v0.position - v2.position;
            glm::vec3 vp2 = hit_point - v2.position;
            c = glm::cross(edge2, vp2);
            if (glm::dot(face_normal, c) < 0) continue; // P is on the right side

            auto hit_ws = glm::vec3(this->getGlobalTransform() * glm::vec4(hit_point, 1));
            auto distance = glm::distance(hit_ws, ray_origin_ls);
            // This ray hits the triangle
            if (ray_cast_hit->distance_from_origin > distance)
            {
                ray_cast_hit->distance_from_origin = distance;
                ray_cast_hit->hit = true;
                ray_cast_hit->hit_local = hit_point;
                ray_cast_hit->hit_normal_local = face_normal;
                ray_cast_hit->hit_world_space = hit_ws;
                ray_cast_hit->hit_normal_world_space = this->getGlobalTransform() * glm::vec4(ray_cast_hit->hit_normal_local, 0);
                ray_cast_hit->object_3d = get_parent();
            }
        }
    }
}

void MeshCollider::visualize_collider()
{
    Collider::visualize_collider();
}

bool MeshCollider::is_in_proximity(glm::vec3 center_ws, float radius)
{
    std::vector<struct_vertex_array*>* vertex_arrays_of_geometry = this->get_vertex_arrays();

    for (unsigned int a = 0; a < vertex_arrays_of_geometry->size(); a++)
    {
        if (is_in_proximity_vertex_array(center_ws, radius, a))
        {
            return true;
        }
    }
    return false;
}

int MeshCollider::get_collider_type()
{
    return 1;
}

bool MeshCollider::is_in_proximity_vertex_array(glm::vec3 center_ws, float radius, unsigned int vertex_array_id)
{
    auto global_inverse = glm::inverse(this->transformGlobal);

    glm::vec3 proximity_center_local = global_inverse * glm::vec4(center_ws, 1);
    std::vector<struct_vertex_array*>* vertex_arrays_of_geometry = this->get_vertex_arrays();


    struct_vertex_array* vertex_array = vertex_arrays_of_geometry->at(vertex_array_id);
    for (unsigned int i = 0; i < vertex_array->indices->size(); i += 3)
    {
        vertex v0 = vertex_array->vertices->at(vertex_array->indices->at(i));
        vertex v1 = vertex_array->vertices->at(vertex_array->indices->at(i + 1));
        vertex v2 = vertex_array->vertices->at(vertex_array->indices->at(i + 2));
        
        if (RayIntersectionHelper::sphere_triangle_intersection(v0.position, v1.position, v2.position,
                                                        proximity_center_local, radius))
        {
            return true;
        }
        
        glm::vec3 face_normal = glm::normalize(v0.normal + v1.normal + v2.normal);
        
        float d = -glm::dot(face_normal, v0.position);
        float t =
            -(glm::dot(face_normal, proximity_center_local) + d);
 
        glm::vec3 hit_point = proximity_center_local + t * face_normal;
        
        auto vec_to_center_global = this->transformGlobal * glm::vec4(hit_point - proximity_center_local, 0.0);
        if (glm::length(vec_to_center_global) > radius) continue;

        auto c = glm::vec3();

        // Edge 0
        glm::vec3 edge0 = v1.position - v0.position;
        glm::vec3 vp0 = hit_point - v0.position;
        c = glm::cross(edge0, vp0);
        if (glm::dot(face_normal, c) < 0) continue; // P is on the right side

        // Edge 1
        glm::vec3 edge1 = v2.position - v1.position;
        glm::vec3 vp1 = hit_point - v1.position;
        c = glm::cross(edge1, vp1);
        if (glm::dot(face_normal, c) < 0) continue; // P is on the right side

        // Edge 2
        glm::vec3 edge2 = v0.position - v2.position;
        glm::vec3 vp2 = hit_point - v2.position;
        c = glm::cross(edge2, vp2);
        if (glm::dot(face_normal, c) < 0) continue; // P is on the right side
        return true;
    }

    return false;
}

bool MeshCollider::is_in_proximity_vertex(float radius, unsigned int v_0, glm::vec3 proximity_center_local,
                                          struct_vertex_array* vertex_array) const
{
    vertex v0 = vertex_array->vertices->at(vertex_array->indices->at(v_0));
    vertex v1 = vertex_array->vertices->at(vertex_array->indices->at(v_0 + 1));
    vertex v2 = vertex_array->vertices->at(vertex_array->indices->at(v_0 + 2));
        
    if (RayIntersectionHelper::sphere_triangle_intersection(v0.position, v1.position, v2.position,
                                                    proximity_center_local, radius))
    {
        return true;
    }
        
    glm::vec3 face_normal = glm::normalize(v0.normal + v1.normal + v2.normal);

    float d = -glm::dot(face_normal, v0.position);
    float t = -(glm::dot(face_normal, proximity_center_local) + d);
 
    glm::vec3 hit_point = proximity_center_local + t * face_normal;
        
    auto vec_to_center_global = this->transformGlobal * glm::vec4(hit_point - proximity_center_local, 0.0);
    if (glm::length(vec_to_center_global) > radius) return false;

    auto c = glm::vec3();

    // Edge 0
    glm::vec3 edge0 = v1.position - v0.position;
    glm::vec3 vp0 = hit_point - v0.position;
    c = glm::cross(edge0, vp0);
    if (glm::dot(face_normal, c) < 0) return false; // P is on the right side

    // Edge 1
    glm::vec3 edge1 = v2.position - v1.position;
    glm::vec3 vp1 = hit_point - v1.position;
    c = glm::cross(edge1, vp1);
    if (glm::dot(face_normal, c) < 0) return false; // P is on the right side

    // Edge 2
    glm::vec3 edge2 = v0.position - v2.position;
    glm::vec3 vp2 = hit_point - v2.position;
    c = glm::cross(edge2, vp2);
    if (glm::dot(face_normal, c) < 0) return false; // P is on the right side
    return true;
}


void MeshCollider::calculate_world_space_bounding_box()
{
    auto temp_bb = StructBoundingBox();
    temp_bb.max = getGlobalTransform() * glm::vec4(vertex_arrays_.at(0)->vertices->at(0).position, 1);
    temp_bb.min = temp_bb.max;

    bounding_box.max = temp_bb.max;
    bounding_box.min = temp_bb.max;
    
    for (auto vertex_array : vertex_arrays_)
    {
        BoundingBoxHelper::get_bounding_box_from_vertex_array(
            &temp_bb,vertex_array,getGlobalTransform());
        BoundingBoxHelper::get_combined_bounding_box(
            &bounding_box,
            &bounding_box,&temp_bb
            );
    }
}

glm::vec3 MeshCollider::calculate_center_of_mass_internal()
{
    auto center_of_mass = glm::vec3(0, 0, 0);
    int vertices = 0;
    for (unsigned int i = 0; i < vertex_arrays_.size(); i++)
    {
        auto vertex_array = this->vertex_arrays_.at(i);
        vertices+=vertex_array->vertices->size();
        for (unsigned int j = 0; j < vertex_array->vertices->size(); j++)
        {
            vertex v = vertex_array->vertices->at(j);
            center_of_mass += v.position;
        }
    }
    return center_of_mass / static_cast<float>(vertices);

}

glm::mat3 MeshCollider::calculate_inertia_tensor_internal()
{
    auto inertia_tensor = glm::mat3(0);
    auto center_of_mass = get_center_of_mass();
    for (unsigned int i = 0; i < vertex_arrays_.size(); i++)
    {
        auto vertex_array = this->vertex_arrays_.at(i);
        for (unsigned int j = 0; j < vertex_array->vertices->size(); j++)
        {
            vertex v = vertex_array->vertices->at(j);
            auto vec_to_point = v.position - center_of_mass;
            inertia_tensor += glm::outerProduct(vec_to_point, vec_to_point);
        }
    }
    return inertia_tensor;
}
