#pragma once
#include <mat4x4.hpp>
#include "../Core/CommonDataStructures/StructVertexArray.h"
#include "../Core/CommonDataStructures/StructBoundingBox.h"

class BoundingBoxHelper
{
public:
    
    static void get_bounding_box_from_vertex_array(
        StructBoundingBox* bb,
        struct_vertex_array* vertex_array,
        glm::mat4x4 transformation_matrix);
    
    static void get_combined_bounding_box(StructBoundingBox *resulting_box, StructBoundingBox *box_a, StructBoundingBox *box_b);

    static void bounding_box_to_sphere(StructBoundingSphere *result_sphere, const StructBoundingBox *bounding_box);

    static glm::vec3 get_center_of_bb(const StructBoundingBox *bounding_box);
    static glm::vec3  get_scale_of_bb(const StructBoundingBox *bounding_box);
    static float  get_max_length_of_bb(const StructBoundingBox *bounding_box);
};
