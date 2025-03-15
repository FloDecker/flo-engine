#include "BoundingBoxHelper.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/string_cast.hpp>

#include "../Core/Scene/Object3D.h"
//#include "../Core/CommonDataStructures/ray_cast_result.h"

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

std::vector<glm::vec3> BoundingBoxHelper::get_edges_of_bounding_box(StructBoundingBox* bb,
                                                                    const glm::mat4x4& transformation_matrix = glm::mat4x4())
{
	std::vector<glm::vec3> result = {
		 bb->min,
		{bb->max.x, bb->min.y, bb->min.z},
		{bb->min.x, bb->max.y, bb->min.z},
		{bb->min.x, bb->min.y, bb->max.z},
		{bb->max.x, bb->max.y, bb->min.z},
		{bb->max.x, bb->min.y, bb->max.z},
		{bb->min.x, bb->max.y, bb->max.z},
		 bb->max
	};

	if (transformation_matrix == glm::mat4x4())
	{
		return result;
	}

	for (int i = 0; i < result.size(); ++i)
	{
		result[i] = transformation_matrix * glm::vec4(result[i], 1);
	}
	
	return result;
}

void BoundingBoxHelper::get_bounding_box_containing_points(StructBoundingBox* bb, const std::vector<glm::vec3>* points)
{
	bb->min = points->at(0);
	bb->max = bb->min;

	for (const auto point : *points)
	{
		if (point.x > bb->max.x) bb->max.x = point.x;
		if (point.y > bb->max.y) bb->max.y = point.y;
		if (point.z > bb->max.z) bb->max.z = point.z;
		if (point.x < bb->min.x) bb->min.x = point.x;
		if (point.y < bb->min.y) bb->min.y = point.y;
		if (point.z < bb->min.z) bb->min.z = point.z;
	}
}

void BoundingBoxHelper::transform_local_bb_to_world_space_axis_aligned(StructBoundingBox* world_space_result,
	StructBoundingBox* local_bb, const glm::mat4x4& transformation_matrix)
{
	const auto p = get_edges_of_bounding_box(local_bb,transformation_matrix);
	get_bounding_box_containing_points(world_space_result,&p);
}

void BoundingBoxHelper::get_combined_bounding_box(StructBoundingBox* resulting_box, const StructBoundingBox* box_a,
                                                  const StructBoundingBox* box_b)
{
	resulting_box->max.x = std::max(box_b->max.x, box_a->max.x);
	resulting_box->max.y = std::max(box_b->max.y, box_a->max.y);
	resulting_box->max.z = std::max(box_b->max.z, box_a->max.z);
	resulting_box->min.x = std::min(box_b->min.x, box_a->min.x);
	resulting_box->min.y = std::min(box_b->min.y, box_a->min.y);
	resulting_box->min.z = std::min(box_b->min.z, box_a->min.z);
}

void BoundingBoxHelper::bounding_box_to_sphere(StructBoundingSphere* result_sphere,
                                               const StructBoundingBox* bounding_box)
{
	result_sphere->radius = glm::distance(bounding_box->max, bounding_box->min);
	result_sphere->center = (bounding_box->max + bounding_box->min) * 0.5f;
}

bool BoundingBoxHelper::are_overlapping(StructBoundingBox* a, StructBoundingBox* b)
{
	//check x
	if (a->max.x < b->min.x || b->max.x < a->min.x)
	{
		return false;
	}
	//check y
	if (a->max.y < b->min.y || b->max.y < a->min.y)
	{
		return false;
	}

	//check z
	if (a->max.z < b->min.z || b->max.z < a->min.z)
	{
		return false;
	}
	return true;
}

bool BoundingBoxHelper::is_in_bounding_box(const StructBoundingBox* bounding_box, const glm::vec3& p,
                                           const float uniform_scale_addition)
{
	return (
		bounding_box->max.x + uniform_scale_addition >= p.x &&
		bounding_box->max.y + uniform_scale_addition >= p.y &&
		bounding_box->max.z + uniform_scale_addition >= p.z &&
		bounding_box->min.x - uniform_scale_addition <= p.x &&
		bounding_box->min.y - uniform_scale_addition <= p.y &&
		bounding_box->min.z - uniform_scale_addition <= p.z
	);
}

bool BoundingBoxHelper::intersects_polygon(const StructBoundingBox* bounding_box, const glm::vec3& v_0,
                                           const glm::vec3& v_1,
                                           const glm::vec3& v_2)
{
	auto center = get_center_of_bb(bounding_box);
	auto half_size = get_scale_of_bb(bounding_box) * 0.5f;

	float v[3][3] = {{v_0.x, v_0.y, v_0.z}, {v_1.x, v_1.y, v_1.z}, {v_2.x, v_2.y, v_2.z}};
	float c[3] = {center.x, center.y, center.z};
	float h[3] = {half_size.x, half_size.y, half_size.z};
	return triBoxOverlap(c, h, v);
}

void BoundingBoxHelper::remove_vertices_not_contained(const StructBoundingBox* bounding_box,
                                                      std::vector<glm::vec3>* vertices)
{
	auto condition = [bounding_box](glm::vec3 x) { return !is_in_bounding_box(bounding_box, x); };
	vertices->erase(std::remove_if(vertices->begin(), vertices->end(), condition), vertices->end());
}

std::array<glm::vec3, 8> BoundingBoxHelper::get_vertices(const StructBoundingBox* bounding_box,
                                                         const glm::mat4& transform_a)
{
	return {
		transform_a * glm::vec4(bounding_box->min.x, bounding_box->min.y, bounding_box->min.z, 1), // 0
		transform_a * glm::vec4(bounding_box->max.x, bounding_box->min.y, bounding_box->min.z, 1), // 1
		transform_a * glm::vec4(bounding_box->min.x, bounding_box->max.y, bounding_box->min.z, 1), // 2
		transform_a * glm::vec4(bounding_box->max.x, bounding_box->max.y, bounding_box->min.z, 1), // 3
		transform_a * glm::vec4(bounding_box->min.x, bounding_box->min.y, bounding_box->max.z, 1), // 4
		transform_a * glm::vec4(bounding_box->max.x, bounding_box->min.y, bounding_box->max.z, 1), // 5
		transform_a * glm::vec4(bounding_box->min.x, bounding_box->max.y, bounding_box->max.z, 1), // 6
		transform_a * glm::vec4(bounding_box->max.x, bounding_box->max.y, bounding_box->max.z, 1) // 7
	};
}

glm::vec3 BoundingBoxHelper::get_center_of_bb(const StructBoundingBox* bounding_box)
{
	return (bounding_box->max + bounding_box->min) * 0.5f;
}

glm::vec3 BoundingBoxHelper::get_scale_of_bb(const StructBoundingBox* bounding_box)
{
	return bounding_box->max - bounding_box->min;
}

glm::vec3 BoundingBoxHelper::get_half_sizes_of_bb(const StructBoundingBox* bounding_box)
{
	return get_scale_of_bb(bounding_box) * 0.5f;
}

float BoundingBoxHelper::get_max_length_of_bb(const StructBoundingBox* bounding_box)
{
	return glm::length(get_scale_of_bb(bounding_box));
}

bool BoundingBoxHelper::ray_axis_aligned_bb_intersection(const StructBoundingBox* bb,
	glm::vec3 ray_origin_ws, glm::vec3 ray_direction_ws, ray_cast_result *intersection)
{
	*intersection = {};
	glm::vec3 normals[6] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
	glm::vec3 points[6] = {
		{bb->max.x, 0, 0}, {bb->min.x, 0, 0}, // +X, -X
		{0, bb->max.y, 0}, {0, bb->min.y, 0}, // +Y, -Y
		{0, 0, bb->max.z}, {0, 0, bb->min.z} // +Z, -Z
	};

	auto nearestT = std::numeric_limits<float>::infinity();
	bool hit = false;
	glm::vec3 hitPoint = {};
	glm::vec3 hitNormal = {};
	for (int i = 0; i < 6; i++)
	{
		float t;
		glm::vec3 intersection_point;
		if (ray_plane_intersection(ray_origin_ws, ray_direction_ws, points[i], normals[i], t, intersection_point))
		{
			if (is_in_bounding_box(bb,intersection_point,0.001))
			{
				if (t < nearestT)
				{
					nearestT = t;
					hitPoint = intersection_point;
					hit = true;
					hitNormal = normals[i];
				}
			}
		}
	}
	intersection->hit = hit;
	if (hit)
	{
		intersection->distance_from_origin = glm::distance(ray_origin_ws, hitPoint);
		intersection->hit_local = hitPoint;
		intersection->hit_world_space = hitPoint;
		intersection->hit_normal_local = hitNormal;
		intersection->hit_normal_world_space = hitNormal;
	}
	return hit;
}



bool BoundingBoxHelper::ray_plane_intersection(
	const glm::vec3& rayOrigin, const glm::vec3& rayDir, // Ray: Origin & Direction
	const glm::vec3& planePoint, const glm::vec3& planeNormal, // Plane: Point & Normal
	float& t, glm::vec3& intersectionPoint) 
{
	float denom = glm::dot(rayDir, planeNormal);

	// Check if the ray is parallel to the plane
	if (fabs(denom) < 1e-6) {
		return false; // No intersection
	}

	t = glm::dot(planePoint - rayOrigin, planeNormal) / denom;

	// If t < 0, the intersection is behind the ray's origin
	if (t < 0) {
		return false; 
	}

	intersectionPoint = rayOrigin + t * rayDir;
	return true; 
}


/********************************************************/

/* AABB-triangle overlap test code                      */

/* by Tomas Akenine-Möller                              */

/* Function: int triBoxOverlap(float boxcenter[3],      */

/*          float boxhalfsize[3],float triverts[3][3]); */

/* History:                                             */

/*   2001-03-05: released the code in its first version */

/*   2001-06-18: changed the order of the tests, faster */

/*                                                      */

/* Acknowledgement: Many thanks to Pierre Terdiman for  */

/* suggestions and discussions on how to optimize code. */

/* Thanks to David Hunt for finding a ">="-bug!         */

/********************************************************/

/*
Copyright 2020 Tomas Akenine-Möller

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define X 0
#define Y 1
#define Z 2
#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2];
#define FINDMINMAX(x0,x1,x2,min,max) \
  min = max = x0;   \
  if(x1<min) min=x1;\
  if(x1>max) max=x1;\
  if(x2<min) min=x2;\
  if(x2>max) max=x2;


/*======================== X-tests ========================*/

#define AXISTEST_X01(a, b, fa, fb)			   \
p0 = a*v0[Y] - b*v0[Z];			       	   \
p2 = a*v2[Y] - b*v2[Z];			       	   \
if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
if(min>rad || max<-rad) return 0;
#define AXISTEST_X2(a, b, fa, fb)			   \
p0 = a*v0[Y] - b*v0[Z];			           \
p1 = a*v1[Y] - b*v1[Z];			       	   \
if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
if(min>rad || max<-rad) return 0;


/*======================== Y-tests ========================*/

#define AXISTEST_Y02(a, b, fa, fb)			   \
p0 = -a*v0[X] + b*v0[Z];		      	   \
p2 = -a*v2[X] + b*v2[Z];	       	       	   \
if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
if(min>rad || max<-rad) return 0;
#define AXISTEST_Y1(a, b, fa, fb)			   \
p0 = -a*v0[X] + b*v0[Z];		      	   \
p1 = -a*v1[X] + b*v1[Z];	     	       	   \
if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
if(min>rad || max<-rad) return 0;


/*======================== Z-tests ========================*/


#define AXISTEST_Z12(a, b, fa, fb)			   \
p1 = a*v1[X] - b*v1[Y];			           \
p2 = a*v2[X] - b*v2[Y];			       	   \
if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
if(min>rad || max<-rad) return 0;
#define AXISTEST_Z0(a, b, fa, fb)			   \
p0 = a*v0[X] - b*v0[Y];				   \
p1 = a*v1[X] - b*v1[Y];			           \
if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
if(min>rad || max<-rad) return 0;


int BoundingBoxHelper::triBoxOverlap(float boxcenter[3], float boxhalfsize[3], float triverts[3][3])

{
	/*    use separating axis theorem to test overlap between triangle and box */
	/*    need to test for overlap in these directions: */
	/*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
	/*       we do not even need to test these) */
	/*    2) normal of the triangle */
	/*    3) crossproduct(edge from tri, {x,y,z}-directin) */
	/*       this gives 3x3=9 more tests */
	float v0[3], v1[3], v2[3];
	float min, max, p0, p1, p2, rad, fex, fey, fez; // -NJMP- "d" local variable removed
	float normal[3], e0[3], e1[3], e2[3];


	/* This is the fastest branch on Sun */
	/* move everything so that the boxcenter is in (0,0,0) */
	SUB(v0, triverts[0], boxcenter);
	SUB(v1, triverts[1], boxcenter);
	SUB(v2, triverts[2], boxcenter);


	/* compute triangle edges */
	SUB(e0, v1, v0); /* tri edge 0 */
	SUB(e1, v2, v1); /* tri edge 1 */
	SUB(e2, v0, v2); /* tri edge 2 */


	/* Bullet 3:  */
	/*  test the 9 tests first (this was faster) */
	fex = fabsf(e0[X]);
	fey = fabsf(e0[Y]);
	fez = fabsf(e0[Z]);
	AXISTEST_X01(e0[Z], e0[Y], fez, fey);
	AXISTEST_Y02(e0[Z], e0[X], fez, fex);
	AXISTEST_Z12(e0[Y], e0[X], fey, fex);


	fex = fabsf(e1[X]);

	fey = fabsf(e1[Y]);

	fez = fabsf(e1[Z]);

	AXISTEST_X01(e1[Z], e1[Y], fez, fey);

	AXISTEST_Y02(e1[Z], e1[X], fez, fex);

	AXISTEST_Z0(e1[Y], e1[X], fey, fex);


	fex = fabsf(e2[X]);

	fey = fabsf(e2[Y]);

	fez = fabsf(e2[Z]);

	AXISTEST_X2(e2[Z], e2[Y], fez, fey);

	AXISTEST_Y1(e2[Z], e2[X], fez, fex);

	AXISTEST_Z12(e2[Y], e2[X], fey, fex);


	/* Bullet 1: */

	/*  first test overlap in the {x,y,z}-directions */

	/*  find min, max of the triangle each direction, and test for overlap in */

	/*  that direction -- this is equivalent to testing a minimal AABB around */

	/*  the triangle against the AABB */


	/* test in X-direction */

	FINDMINMAX(v0[X], v1[X], v2[X], min, max);

	if (min > boxhalfsize[X] || max < -boxhalfsize[X]) return 0;


	/* test in Y-direction */

	FINDMINMAX(v0[Y], v1[Y], v2[Y], min, max);

	if (min > boxhalfsize[Y] || max < -boxhalfsize[Y]) return 0;


	/* test in Z-direction */

	FINDMINMAX(v0[Z], v1[Z], v2[Z], min, max);

	if (min > boxhalfsize[Z] || max < -boxhalfsize[Z]) return 0;


	/* Bullet 2: */

	/*  test if the box intersects the plane of the triangle */

	/*  compute plane equation of triangle: normal*x+d=0 */

	CROSS(normal, e0, e1);

	// -NJMP- (line removed here)

	if (!planeBoxOverlap(normal, v0, boxhalfsize)) return 0; // -NJMP-


	return 1; /* box and triangle overlaps */
}


int BoundingBoxHelper::planeBoxOverlap(float normal[3], float vert[3], float maxbox[3]) // -NJMP-

{
	int q;
	float vmin[3], vmax[3], v;
	for (q = X; q <= Z; q++)
	{
		v = vert[q]; // -NJMP-
		if (normal[q] > 0.0f)
		{
			vmin[q] = -maxbox[q] - v; // -NJMP-
			vmax[q] = maxbox[q] - v; // -NJMP-
		}
		else
		{
			vmin[q] = maxbox[q] - v; // -NJMP-
			vmax[q] = -maxbox[q] - v; // -NJMP-
		}
	}
	if (DOT(normal, vmin) > 0.0f) return 0; // -NJMP-
	if (DOT(normal, vmax) >= 0.0f) return 1; // -NJMP-
	return 0;
}

float BoundingBoxHelper::penetration_depth(glm::vec3 axis_to_check, glm::vec3 half_sizes_a,
                                           glm::vec3 axis_world_space_a[3], glm::vec3 half_sizes_b,
                                           glm::vec3 axis_world_space_b[3], glm::vec3 center_distance)
{
	float proj_a = project_cube_on_axis(axis_to_check, half_sizes_a, axis_world_space_a);
	float proj_b = project_cube_on_axis(axis_to_check, half_sizes_b, axis_world_space_b);
	float dist = fabs(glm::dot(center_distance, axis_to_check));

	return  (proj_a + proj_b) - dist;
}

struct_intersection BoundingBoxHelper::are_intersecting(const StructBoundingBox* bounding_box_a,
                                                     const StructBoundingBox* bounding_box_b,
                                                     const glm::mat4& transform_a, const glm::mat4& transform_b)
{
	struct_intersection return_struct;

	glm::vec3 a_x = transform_a * vec_x_w_vector;
	glm::vec3 a_y = transform_a * vec_y_w_vector;
	glm::vec3 a_z = transform_a * vec_z_w_vector;
	glm::vec3 a_normals[3] = {a_x, a_y, a_z};

	glm::vec3 b_x = transform_b * vec_x_w_vector;
	glm::vec3 b_y = transform_b * vec_y_w_vector;
	glm::vec3 b_z = transform_b * vec_z_w_vector;
	glm::vec3 b_normals[3] = {b_x, b_y, b_z};

	glm::vec3 cross_normals[9] = {
		glm::cross(a_x, b_x), glm::cross(a_x, b_y), glm::cross(a_x, b_z),
		glm::cross(a_y, b_x), glm::cross(a_y, b_y), glm::cross(a_y, b_z),
		glm::cross(a_z, b_x), glm::cross(a_z, b_y), glm::cross(a_z, b_z)
	};
	

	float min_penetration = FLT_MAX;
	glm::vec3 best_axis;
	glm::vec3 half_sizes_a = get_half_sizes_of_bb(bounding_box_a);
	glm::vec3 half_sizes_b = get_half_sizes_of_bb(bounding_box_b);

	glm::vec3 axis_world_space_a[3] = {
		transform_a * vec_x_w_vector,
		transform_a * vec_y_w_vector,
		transform_a * vec_z_w_vector
	};


	glm::vec3 axis_world_space_b[3] = {
		transform_b * vec_x_w_vector,
		transform_b * vec_y_w_vector,
		transform_b * vec_z_w_vector
	};

	glm::vec3 center_distance =
		transform_a * glm::vec4(get_center_of_bb(bounding_box_a), 1) -
		transform_b * glm::vec4(get_center_of_bb(bounding_box_b), 1);

	unsigned int intersection_type = 0;


	//type 0 = face-face 
	for (int i = 0; i < 3; i++)
	{
		auto axis = normalize(a_normals[i]);
		float penetration = penetration_depth(axis, half_sizes_a,
		                                      axis_world_space_a, half_sizes_b,
		                                      axis_world_space_b, center_distance);
		if (penetration < 0) return {}; // Separating axis found, no collision

		//check normals of a
		if (penetration < min_penetration)
		{
			intersection_type = 0;
			min_penetration = penetration;
			best_axis = axis;
		}
	}


	//type 1 = face-face 
	for (int i = 0; i < 3; i++)
	{
		auto axis = normalize(b_normals[i]);
		float penetration = penetration_depth(axis, half_sizes_a,
											  axis_world_space_a, half_sizes_b,
											  axis_world_space_b, center_distance);
		if (penetration < 0) return {}; // Separating axis found, no collision

		//check normals of a
		if (penetration < min_penetration)
		{
			intersection_type = 1;
			min_penetration = penetration;
			best_axis = axis;
		}
	}

	//type 2 = face-vertex 
	for (int i = 0; i < 9; i++)
	{
		auto axis = normalize(cross_normals[i]);
		float penetration = penetration_depth(axis, half_sizes_a,
											  axis_world_space_a, half_sizes_b,
											  axis_world_space_b, center_distance);
		if (penetration < 0) return {}; // Separating axis found, no collision

		//check normals of a
		if (penetration < min_penetration)
		{
			intersection_type = 2;
			min_penetration = penetration;
			best_axis = axis;
		}
	}

	printf("intersection_type %i \n", intersection_type);

	if (glm::dot(center_distance, best_axis) < 0) best_axis = -best_axis;
	return_struct.intersected = true;
	return_struct.collision_normal = best_axis;

	float estimated_depth = fabs(glm::dot(center_distance, best_axis));
	return_struct.collision_point = glm::vec3(
		transform_b * glm::vec4(get_center_of_bb(bounding_box_b) + best_axis * (estimated_depth * 0.5f), 1));
	return return_struct;
}


float BoundingBoxHelper::project_cube_on_axis(glm::vec3 axis, glm::vec3 half_sizes, glm::vec3 bb_axis[3])
{
	return fabs(half_sizes.x * dot(bb_axis[0], axis)) +
		fabs(half_sizes.y * dot(bb_axis[1], axis)) +
		fabs(half_sizes.z * dot(bb_axis[2], axis));
}
