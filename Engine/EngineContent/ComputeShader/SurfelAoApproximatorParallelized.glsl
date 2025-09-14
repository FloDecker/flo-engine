#version 430 core
#define PI 3.14159265359
#define PiOver2  1.57079632679489661923f
#define PiOver4  0.78539816339744830961f

#define OCTREE_TOTOAL_EXTENSION 512

#define ITERATIONS 64
#define MAX_OCTREE_RAYTRACING_STEPS 1024
#define MAX_VALUE 10000000.0f

#define BITMASK_SURFEL_AMOUNT 0x00FFFFFFu

struct Surfel {
    vec4 mean_r;
    vec4 normal;
    vec4 albedo;
    vec4 radiance_ambient;//radiance without surface irradiance and direct light 
    vec4 radiance_direct_and_surface;//radiance contribution from direct light and surface
    uint[8] copy_locations;//global adresses where this exact surfel can be found
};

struct OctreeElement
{
    uint surfels_at_layer_amount;
    uint surfels_at_layer_pointer;
    uint next_layer_surfels_pointer[8];
};

struct Ray {
    vec3 origin;
    vec3 direction;
    vec3 inverse_direction;
    float max_length;
};

struct AllocationMetadata{
    uint surfel_bucket_pointer;
    uint surfel_octree_pointer;
    uint octree_pointer_update_index;
    uint debug_int_32;
};

layout (local_size_x = ITERATIONS, local_size_y = 1, local_size_z = 1) in;

//SURFEL BACKBUFFER
layout(std430, binding = 3) buffer SurfelBuffer {
    Surfel surfels[];
};
//OCTREE BACKBUFFER
layout(std430, binding = 5) readonly buffer OctreeBuffer {
    OctreeElement octreeElements[];
};

layout(std430, binding = 8) buffer UpdatedOctreeElements {
    uint updatedIds[];
};

layout(std430, binding = 2) buffer AllocationMetadataBuffer {
    AllocationMetadata allocationMetadata[];
};

uniform int offset_id;
uniform int calculation_level;
uniform uvec3 pos_ws_start;

shared vec3 ray_trace_results[ITERATIONS];
shared Surfel surfel;
shared bool has_surfel;
shared uint surfel_pointer;

vec3 get_ao_color(){
    return vec3(80.0/255.0, 156.0/255.0, 250.0/255.0) * 0.7;
}

uint get_surfel_amount(uint i) {
    return i & BITMASK_SURFEL_AMOUNT;
}

bool is_child_octree_bit_set_at(uint i, uint pos)
{
    return (i & (1u << 31-pos)) != 0;
}

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec2 rand2(vec2 co) {
    return vec2(rand(co), rand(co + 1.23));
}

uint get_next_octree_index_(uvec3 center, uvec3 pos)
{
    return (uint(pos.x >= center.x) << 2) |
    (uint(pos.y >= center.y) << 1) |
    (uint(pos.z >= center.z) << 0);
}

bool is_ws_pos_contained_in_bb(vec3 pos, vec3 bb_min, vec3 extension) {
    vec3 bb_max = bb_min + extension;
    return
    pos.x <= bb_max.x && pos.x >= bb_min.x &&
    pos.y <= bb_max.y && pos.y >= bb_min.y &&
    pos.z <= bb_max.z && pos.z >= bb_min.z;
}

//from https://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations#ConcentricSampleDisk
vec2 ConcentricSampleDisk(vec2 u) {
    vec2 uOffset = 2.f * u - vec2(1.0f);
    if (uOffset.x == 0.0f && uOffset.y == 0.0f) return vec2(0.0f);
    float theta, r;
    bool x_over_y = abs(uOffset.x) > abs(uOffset.y);
    r = x_over_y?uOffset.x:uOffset.y;
    theta = x_over_y ? (PiOver4 * (uOffset.y / uOffset.x)) : (PiOver2 - PiOver4 * (uOffset.x / uOffset.y));
    return r * vec2(cos(theta), sin(theta));
}

vec3 CosineSampleHemisphere(vec2 u) {
    vec2 d = ConcentricSampleDisk(u);
    float z = sqrt(max(0.0f, 1.0f - d.x * d.x - d.y * d.y));
    return vec3(d.x, d.y, z);
}

//from https://iquilezles.org/articles/intersectors/
bool ray_surfel_intersection(Surfel s, Ray r, out vec3 hit_location)
{
    if (dot(s.normal.xyz, r.direction) > 0) return false;

    vec3  o = r.origin - s.mean_r.xyz;
    float t = -dot(s.normal.xyz, o)/dot(r.direction, s.normal.xyz);
    vec3  q = o + r.direction * t;
    hit_location = r.origin + r.direction*t;
    return (dot(q, q)<s.mean_r.w * s.mean_r.w) && (t >= 0);
}


//adapted from https://iquilezles.org/articles/intersectors/
// axis aligned box centered at the origin, with size boxSize
bool boxIntersection(in Ray r, float boxSize, vec3 boxStartWS, out float distance, out float distanceNear)
{

    vec3 origin = r.origin - boxStartWS - boxSize * 0.5f;// transform the origin 
    vec3 n = r.inverse_direction * origin;// can precompute if traversing a set of aligned boxes
    vec3 k = abs(r.inverse_direction)*boxSize * 0.5f;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = max(max(t1.x, t1.y), t1.z);
    float tF = min(min(t2.x, t2.y), t2.z);

    distanceNear = tN;
    distance = (tN < 0.0) ? 0.0 : tN;
    return tN<tF && tF>0.0;
}

void insert_sorted(inout uint id_array[8], inout float distance_array[8], inout int count, uint new_id, float new_distance) {

    distance_array[7] = new_distance;
    id_array[7] = new_id;

    #pragma unroll
    for (int x = 7; x > 0; x--) {
        float value_right = distance_array[x];
        float value_left = distance_array[x-1];
        uint id_right = id_array[x];
        uint id_left = id_array[x-1];
        //right value should be larger than left one

        //float smaller = min(value_left,value_right);

        bool swap_values = value_left >= value_right;//repace if to reduce branching

        distance_array[x] = swap_values?value_left:distance_array[x];
        distance_array[x-1] = swap_values?value_right:distance_array[x-1];

        //also swap ids
        id_array[x] = swap_values?id_left:id_array[x];
        id_array[x - 1] = swap_values?id_right:id_array[x - 1];
    }
    count++;

}


//returns sorting order low -> high
int get_ordered_child_traversal(float extension_parent, vec3 parent_min, Ray r, out uint[8] ordered_ids){

    float child_size = extension_parent * 0.5f;
    int length_ids = 0;

    uint[8] id_array = uint[8](0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
    float[8] distance_array = float[8](MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE);

    #pragma unroll
    for (int i = 0; i < 8; i++) {

        vec3 offset = vec3(
        (i & (1<<2)) != 0 ? 1.0 : 0.0,
        (i & (1<<1)) != 0 ? 1.0 : 0.0,
        (i & (1<<0)) != 0 ? 1.0 : 0.0
        );

        vec3 child_min = parent_min + offset * child_size;
        float d;
        float _;
        if (boxIntersection(r, child_size, child_min, d, _)){
            //the ray intersects the aabb
            //bubble sort distance
            insert_sorted(id_array, distance_array, length_ids, i, d);
        }


    }
    ordered_ids = id_array;
    return length_ids;
}

struct stack_item {
    uint octree_pointer;
    float extension;
    vec3 min_pos;
};

bool traverseHERO(Ray ray, out vec3 c, out float d) {
    const int MAX_STACK = 64;
    stack_item stack[MAX_STACK];

    stack[0].octree_pointer = 0;
    stack[0].extension = OCTREE_TOTOAL_EXTENSION;
    stack[0].min_pos = vec3(-OCTREE_TOTOAL_EXTENSION*0.5f);

    int stackPtr = 0;

    float closest_hit = MAX_VALUE;
    vec3 current_best_hit;
    bool has_hit = false;

    for (int tries = 0; tries < MAX_OCTREE_RAYTRACING_STEPS; tries++) {

        OctreeElement o = octreeElements[stack[stackPtr].octree_pointer];
        float current_bucket_size = stack[stackPtr].extension;
        vec3 current_bucket_min = stack[stackPtr].min_pos;


        float near_distance;
        float dist;
        boxIntersection(ray, current_bucket_size, current_bucket_min, dist, near_distance);

        bool abort_iteration = stackPtr < 0 || dist > closest_hit;
        tries = abort_iteration ? MAX_OCTREE_RAYTRACING_STEPS : tries;


        //check if there are surfels hit on the current layer 
        uint surfels_amount = get_surfel_amount(o.surfels_at_layer_amount);

        uint surfle_data_pointer = o.surfels_at_layer_pointer;
        for (int i = 0; i < surfels_amount; i++) {
            Surfel s = surfels[surfle_data_pointer + i];
            vec3 hit_location;
            if (ray_surfel_intersection(s, ray, hit_location)) {
                d = distance(hit_location, ray.origin);
                if (d < closest_hit) {
                    c = s.radiance_direct_and_surface.xyz + smoothstep(2500, 4000, s.radiance_ambient.w) * s.radiance_ambient.xyz * s.albedo.rgb;
                    closest_hit = d;
                    current_best_hit = hit_location;
                    has_hit =true;
                }
            }
        }

        stackPtr--;

        float children_size = current_bucket_size * 0.5f;

        uint[8]ordered_ids;

        //returns sorting from lowest distance to hightest
        int intersected_children = get_ordered_child_traversal(current_bucket_size, current_bucket_min, ray, ordered_ids);
        for (int i = intersected_children - 1; i >= 0; i--) {
            uint id = ordered_ids[i];

            if (!is_child_octree_bit_set_at(o.surfels_at_layer_amount, id)) continue;
            uint childIndex = o.next_layer_surfels_pointer[id];

            vec3 offset = vec3(
            (id & (1u<<2)) != 0 ? 1.0 : 0.0,
            (id & (1u<<1)) != 0 ? 1.0 : 0.0,
            (id & (1u<<0)) != 0 ? 1.0 : 0.0
            );
            vec3 child_min = current_bucket_min + offset * children_size;
            if (stackPtr < MAX_STACK) {
                stackPtr++;
                stack[stackPtr].octree_pointer = childIndex;
                stack[stackPtr].extension = children_size;
                stack[stackPtr].min_pos = child_min;
            }
        }
    }
    return has_hit;
}


void getTangentBasis(vec3 normal, out vec3 tangent, out vec3 bitangent) {
    vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    tangent = normalize(cross(up, normal));
    bitangent = cross(normal, tangent);
}

vec3 approx_lighting_for_pos(vec3 pos, float radius, vec3 normal, vec4 color_sampled_old, vec3 albedo) {
    vec3 color_pre = color_sampled_old.rgb;
    float sampled_pre = color_sampled_old.a;
    vec2 random_seed = pos.xy*pos.z + sampled_pre + gl_LocalInvocationID.x;
    vec3 d = CosineSampleHemisphere(rand2(random_seed));
    vec3 tangent;
    vec3 bitangent;


    getTangentBasis(normal, tangent, bitangent);

    vec3 s = normalize(normal * d.z  + d.y * bitangent + d.x * tangent);
    s*= vec3(1.0);
    Ray r;
    r.origin = pos;
    r.direction = s;
    r.inverse_direction = 1.0f/s;
    r.max_length = 0.0f;

    vec3 c;
    float dist;

    vec3 final_sample_color = (traverseHERO(r, c, dist)? c : get_ao_color());
    return final_sample_color;
}


//returns the pointer to the surfels in octree node at pos x,y,z and level 
bool get_surfe_pointer_at_octree_pos(uint level, uvec3 pos, out uint pointer, out vec3 metadata) {
    uint current_element_index = 0;
    uvec3 last_min = uvec3(0);
    uint last_size = 1 << level;

    for (int i = 0; i < level; i ++) {
        OctreeElement o = octreeElements[current_element_index];
        last_size = last_size >> 1;// integer divison by 2

        uvec3 center = last_min + last_size;

        uint index = get_next_octree_index_(center, pos);

        if (!is_child_octree_bit_set_at(o.surfels_at_layer_amount, index)) {
            return false;
        }

        current_element_index = o.next_layer_surfels_pointer[index];

        uvec3 offset = uvec3(
        (index & uint(1<<2)) != 0 ? 1 : 0,
        (index & uint(1<<1)) != 0 ? 1 : 0,
        (index & uint(1<<0)) != 0 ? 1 : 0
        );
        last_min += offset * last_size;
    }

    pointer = current_element_index;
    return true;
}


//this method inserts the poitner of th updated octree node into the update array
void insert_update_info(uint p) {
    uint p_copy = p;
    uint free_spot = atomicAdd(allocationMetadata[0].octree_pointer_update_index, 1);
    atomicExchange(updatedIds[free_spot], p_copy);
}

void main() {
    bool is_leader = gl_LocalInvocationID.x == 0u;
    uint p;
    vec3 metadata = vec3(0.0);
    const uint level = calculation_level;

    float node_size_at_level = OCTREE_TOTOAL_EXTENSION / float(1<<level);

    uvec3 center = pos_ws_start;
    vec3 bb_min = vec3(-OCTREE_TOTOAL_EXTENSION * 0.5f) + center * node_size_at_level;
    vec3 bb_extension = vec3(node_size_at_level);
    
    if (is_leader) {
        has_surfel = false;
    }

    //only the leader looks if the surfel exisit
    if (is_leader && get_surfe_pointer_at_octree_pos(level, center, p, metadata)){
        OctreeElement o = octreeElements[p];

        //insert this pointer into the list of changed surfels
        uint id = gl_WorkGroupID.x;
        uint surfels_amount = get_surfel_amount(o.surfels_at_layer_amount);
        if (id < surfels_amount) {
            uint surfle_data_pointer = o.surfels_at_layer_pointer;
            surfel_pointer = surfle_data_pointer + id;
            surfel = surfels[surfel_pointer];
            vec3 surfel_pos = surfel.mean_r.xyz;
            if (is_ws_pos_contained_in_bb(surfel_pos, bb_min, bb_extension)) {
                has_surfel = true;
                insert_update_info(p);
            }
        }
    }
    //wait for leader thread to find surfel
    barrier();

    if (has_surfel) {
        vec3 ray_trace_result = approx_lighting_for_pos(surfel.mean_r.xyz + surfel.normal.xyz * 0.1f, surfel.mean_r.w, surfel.normal.xyz, surfel.radiance_ambient, surfel.albedo.rgb);
        ray_trace_results[gl_LocalInvocationID.x] = ray_trace_result;
    } else {
        return;
    }

    //wait for all threads to run ray tracing
    barrier();

    //leader thread continious
    if (!is_leader) {
        return;
    }
    vec3 result_accumuluation = vec3(0.0);

    #pragma unroll
    for (int i = 0; i < ITERATIONS; i++) {
        result_accumuluation+=ray_trace_results[i];
    }
    vec3 old_estimate = surfel.radiance_ambient.rgb;
    vec3 current_estimate = result_accumuluation / ITERATIONS;

    float old_estimate_samples = surfel.radiance_ambient.w;

    const float delta = 0.5;
    float estimate_delta = distance(old_estimate, current_estimate);
    float iteration_weigth = 1.0 - min(estimate_delta, 1.0) * 0.8f;


    float total_samples = max(ITERATIONS + old_estimate_samples, 1);

    float current_estimate_weight = ITERATIONS / total_samples;
    float old_estimate_weigth = old_estimate_samples / total_samples;

    vec3 new_estimate =  old_estimate * old_estimate_weigth + current_estimate * current_estimate_weight;


    vec4 final_ao_color = vec4(new_estimate, total_samples);
    surfels[surfel_pointer].radiance_ambient = final_ao_color;

    for (int i  = 0; i < 8; i++) {
        uint location = surfel.copy_locations[i];
        surfels[location].radiance_ambient = final_ao_color;
    }
}
