#pragma once
#include <mat4x4.hpp>
#include "../Core/CommonDataStructures/StructVertexArray.h"
#include "../Core/CommonDataStructures/StructBoundingBox.h"
#include "../Core/CommonDataStructures/struct_intersection.h"
#include "../Core/CommonDataStructures/ray_cast_result.h"
#include <array>


class BoundingBoxHelper
{
public:

    //returns the smallest bounding box that contains all vertices
    static void get_bounding_box_from_vertex_array(
        StructBoundingBox* bb,
        struct_vertex_array* vertex_array,
        glm::mat4x4 transformation_matrix);

    static std::vector<glm::vec3> get_edges_of_bounding_box(StructBoundingBox* bb, const glm::mat4x4& transformation_matrix);
	
	static void get_bounding_box_containing_points(StructBoundingBox* bb, const std::vector<glm::vec3> *points);

	static void transform_local_bb_to_world_space_axis_aligned(StructBoundingBox* world_space_result,StructBoundingBox* local_bb, const glm::mat4x4& transformation_matrix ); 

	//takes two bounding boxes and returns the smallest that contains both
    static void get_combined_bounding_box(StructBoundingBox *resulting_box, const StructBoundingBox *box_a, const StructBoundingBox *box_b);

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

    static bool ray_axis_aligned_bb_intersection(const ::StructBoundingBox* bb, glm::vec3 ray_origin_ws,
                                                 glm::vec3 ray_direction_ws, ray_cast_result* intersection);
    static bool ray_plane_intersection(
	    const glm::vec3& rayOrigin, const glm::vec3& rayDir,
	    const glm::vec3& planePoint, const glm::vec3& planeNormal,
	    float& t, glm::vec3& intersectionPoint);
	
    //intersection test (code by Tomas Akenine-Möller)
    static int triBoxOverlap(float boxcenter[3], float boxhalfsize[3], float triverts[3][3]);
    static int planeBoxOverlap(float normal[3], float vert[3], float maxbox[3]) ;

    //Separating axis test
    static struct_intersection are_intersecting(const StructBoundingBox *bounding_box_a, const StructBoundingBox *bounding_box_b, const glm::mat4& transform_a, const glm::mat4& transform_b);
    static float project_cube_on_axis(glm::vec3 axis, glm::vec3 half_sizes, glm::vec3 bb_axis[3]);

    static float penetration_depth(glm::vec3 axis_to_check, glm::vec3 half_sizes_a,
                                   glm::vec3 axis_world_space_a[3], glm::vec3 half_sizes_b,
                                   glm::vec3 axis_world_space_b[3], glm::vec3 center_distance);
};
