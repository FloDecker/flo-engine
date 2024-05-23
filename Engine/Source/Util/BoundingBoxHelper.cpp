#include "BoundingBoxHelper.h"

void BoundingBoxHelper::get_bounding_box_from_vertex_array(StructBoundingBox* bb, struct_vertex_array* vertex_array,
                                                            glm::mat4x4 transformation_matrix)
{
    bb->min = transformation_matrix * glm::vec4(vertex_array->vertices->at(0).position, 1);
    bb->max = bb->max;

    for (unsigned int i = 0; i < vertex_array->vertices->size(); i++)
    {
        const auto v_t = transformation_matrix * glm::vec4(vertex_array->vertices->at(i).position, 1);
        if (v_t.x > bb->max.x) bb->max.x = v_t.x;
        if (v_t.y > bb->max.y) bb->max.y = v_t.y;
        if (v_t.z > bb->max.z) bb->max.z = v_t.z;
        if (v_t.x < bb->min.x) bb->min.x = v_t.x;
        if (v_t.y < bb->min.y) bb->min.y = v_t.y;
        if (v_t.z < bb->min.z) bb->min.z = v_t.z;
    }
}

void BoundingBoxHelper::get_combined_bounding_box(StructBoundingBox* resulting_box, StructBoundingBox* box_a,
                                                  StructBoundingBox* box_b)
{
    resulting_box->max.x = std::max(box_b->max.x, box_a->max.x);
    resulting_box->max.y = std::max(box_b->max.y, box_a->max.y);
    resulting_box->max.z = std::max(box_b->max.z, box_a->max.z);
    resulting_box->min.x = std::min(box_b->min.x, box_a->min.x);
    resulting_box->min.y = std::min(box_b->min.y, box_a->min.y);
    resulting_box->min.z = std::min(box_b->min.z, box_a->min.z);
}

void BoundingBoxHelper::bounding_box_to_sphere(StructBoundingSphere* result_sphere, const StructBoundingBox* bounding_box)
{
    result_sphere->radius = glm::distance(bounding_box->max, bounding_box->min);
    result_sphere->center = (bounding_box->max + bounding_box->min) * 0.5f;
}

glm::vec3 BoundingBoxHelper::get_center_of_bb(const StructBoundingBox* bounding_box)
{
    return (bounding_box->max + bounding_box->min) * 0.5f;
}

glm::vec3 BoundingBoxHelper::get_scale_of_bb(const StructBoundingBox* bounding_box)
{
    return bounding_box->max - bounding_box->min;
}

float BoundingBoxHelper::get_max_length_of_bb(const StructBoundingBox* bounding_box)
{
    return glm::length(get_scale_of_bb(bounding_box));
}

