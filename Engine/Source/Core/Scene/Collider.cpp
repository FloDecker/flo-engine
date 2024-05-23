#include "Collider.h"

Collider::Collider(GlobalContext* global_context): Object3D(global_context)
{
    
}

void Collider::check_collision(glm::vec3 ray_origin_ws, glm::vec3 ray_direction_ws, float ray_length,
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


//Mesh Collider
MeshCollider::MeshCollider(GlobalContext* global_context,
                           const std::vector<struct_vertex_array*>& vertex_arrays): Collider(global_context)
{
    vertex_arrays_ = vertex_arrays;
}

MeshCollider::MeshCollider(GlobalContext* global_context, Mesh3D* mesh): Collider(global_context)
{
    for (auto vertex_array : mesh->get_mesh()->vertexArrays)
    {
        vertex_arrays_.push_back(vertex_array->get_vertex_array());
    }
    mesh->addChild(this);
    name = "collider of " + mesh->name;
}

std::vector<struct_vertex_array*>* MeshCollider::get_vertex_arrays()
{
    return &vertex_arrays_;
}

void MeshCollider::check_collision(glm::vec3 ray_origin_ws, glm::vec3 ray_direction_ws, float ray_length,
                                   bool ignore_back_face,
                                   RayCastHit* ray_cast_hit)
{
    auto global_inverse = glm::inverse(this->transformGlobal);
    glm::vec4 ray_cast_origin_vec4_local = global_inverse * glm::vec4(ray_origin_ws, 1);
    glm::vec3 ray_cast_origin_vec3_local = glm::vec3(ray_cast_origin_vec4_local);
    std::vector<struct_vertex_array*>* vertex_arrays_of_geometry = this->get_vertex_arrays();

    glm::vec4 ray_cast_direction_vec4_local = global_inverse * glm::vec4(ray_direction_ws, 0);
    glm::vec3 ray_cast_direction_vec3_local = glm::vec3(ray_cast_direction_vec4_local);

    for (unsigned int a = 0; a < vertex_arrays_of_geometry->size(); a++)
    {
        struct_vertex_array* vertex_array = vertex_arrays_of_geometry->at(a);
        for (unsigned int i = 0; i < vertex_array->indices->size(); i += 3)
        {
            vertex v0 = vertex_array->vertices->at(vertex_array->indices->at(i));
            vertex v1 = vertex_array->vertices->at(vertex_array->indices->at(i + 1));
            vertex v2 = vertex_array->vertices->at(vertex_array->indices->at(i + 2));


            glm::vec3 face_normal = glm::normalize(v0.normal + v1.normal + v2.normal);

            if (ignore_back_face && glm::dot(face_normal, ray_cast_direction_vec3_local) > 0) continue;

            float d = -glm::dot(face_normal, v0.position);
            float t =
                -((glm::dot(face_normal, ray_cast_origin_vec3_local) + d) /
                    glm::dot(face_normal, ray_cast_direction_vec3_local));

            glm::vec3 hit_point = ray_cast_origin_vec3_local + t * ray_cast_direction_vec3_local;
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
            auto distance = glm::distance(hit_ws, ray_origin_ws);
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

bool MeshCollider::is_in_proximity(glm::vec3 center_ws, float radius)
{
    auto global_inverse = glm::inverse(this->transformGlobal);
    
    glm::vec3 proximity_center_local = global_inverse * glm::vec4(center_ws, 1);
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


            float d = -glm::dot(face_normal, v0.position);
            float t =
                -(glm::dot(face_normal, proximity_center_local) + d);

            glm::vec3 hit_point = proximity_center_local + t * face_normal;
            auto vec_to_center_global = this->transformGlobal * glm::vec4(hit_point-proximity_center_local,0.0);
            
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
    }
    return false;
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
