#pragma once
#include <mat4x4.hpp>
#include "../Core/CommonDataStructures/StructVertexArray.h"
#include "../Core/CommonDataStructures/StructBoundingBox.h"

class BoundingBoxHelper
{
public:
    
    static void get_bounding_box_from_vertex_arrays(
        StructBoundingBox* bb,
        struct_vertex_array* vertex_arrays,
        glm::mat4x4 transformation_matrix);

    static void get_combined_bounding_box(StructBoundingBox *resulting_box, StructBoundingBox *box_a, StructBoundingBox *box_b);
};
