#pragma once
#include <mat4x4.hpp>
#include "../Core/CommonDataStructures/StructVertexArray.h"
#include "../Core/CommonDataStructures/StructBoundingBox.h"
#include "../Core/CommonDataStructures/struct_collision.h"
#include <array>
class BoundingBoxHelper
{
public:

    //returns the smallest bounding box that contains all vertices
    static void get_bounding_box_from_vertex_array(
        StructBoundingBox* bb,
        struct_vertex_array* vertex_array,
        glm::mat4x4 transformation_matrix);

    //takes two bounding boxes and contains the smallest that contains both
    static void get_combined_bounding_box(StructBoundingBox *resulting_box, StructBoundingBox *box_a, StructBoundingBox *box_b);

    //takes a bounding box and returns the smallest spherical bounding box 
    static void bounding_box_to_sphere(StructBoundingSphere *result_sphere, const StructBoundingBox *bounding_box);

    //checks if two bounding boxes are overlapping
    static bool are_overlapping(StructBoundingBox *a, StructBoundingBox *b);

    //checks if a point is contained in a bounding box
    static bool is_in_bounding_box(const StructBoundingBox *bounding_box, const glm::vec3& p, const float uniform_scale_addition = 0.0f);

    //checks if a polygon intersects with a bounding box
    static bool intersects_polygon(const StructBoundingBox* bounding_box, const glm::vec3& v_0, const glm::vec3& v_1, const glm::vec3& v_2);

    //removes all vertices that aren't contained in the bounding box
    static void remove_vertices_not_contained(const StructBoundingBox* bounding_box, std::vector<glm::vec3> *vertices);

    //bb edges
    static std::array<glm::vec3,8> get_vertices(const StructBoundingBox* bounding_box, const glm::mat4& transform_a);
        
    static glm::vec3 get_center_of_bb(const StructBoundingBox *bounding_box);
    static glm::vec3  get_scale_of_bb(const StructBoundingBox *bounding_box);
    static glm::vec3  get_half_sizes_of_bb(const StructBoundingBox *bounding_box);
    static float  get_max_length_of_bb(const StructBoundingBox *bounding_box);

    //intersection test (code by Tomas Akenine-Möller)
    static int triBoxOverlap(float boxcenter[3], float boxhalfsize[3], float triverts[3][3]);
    static int planeBoxOverlap(float normal[3], float vert[3], float maxbox[3]) ;

    //Separating axis test
    static struct_collision are_intersecting(const StructBoundingBox *bounding_box_a, const StructBoundingBox *bounding_box_b, const glm::mat4& transform_a, const glm::mat4& transform_b);
    static float project_cube_on_axis(glm::vec3 axis, glm::vec3 half_sizes, glm::vec3 bb_axis[3]);
};
