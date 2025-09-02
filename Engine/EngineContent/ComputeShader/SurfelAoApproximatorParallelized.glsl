#version 430 core
#define PI 3.14159265359
#define OCTREE_TOTOAL_EXTENSION 512
#define ITERATIONS 4
#define MAX_OCTREE_RAYTRACING_STEPS 1024
struct Surfel {
    vec4 mean_r;
    vec4 normal;
    vec4 radiance_ambient; //radiance without surface irradiance and direct light 
    vec4 radiance_direct_and_surface; //radiance contribution from direct light and surface
    uint[8] copy_locations; //global adresses where this exact surfel can be found
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

//SURFEL BACKBUFFER
layout(std430, binding = 3) buffer SurfelBuffer {
    Surfel surfels[];
};
//OCTREE BACKBUFFER
layout(std430, binding = 5) buffer OctreeBuffer {
    OctreeElement octreeElements[];
};

layout(std430, binding = 8) buffer UpdatedOctreeElements {
    uint updatedIds[];
};

layout(std430, binding = 2) buffer AllocationMetadataBuffer {
    AllocationMetadata allocationMetadata[];
};

layout (local_size_x = ITERATIONS, local_size_y = 1, local_size_z = 1) in;

uniform int offset_id;
uniform int calculation_level;
uniform uvec3 pos_ws_start;

vec3 get_ao_color(){
    return vec3(80.0/255.0,156.0/255.0,250.0/255.0) * 0.5;
}



uint bitmask_surfel_amount = 0x00FFFFFF;


uint get_surfel_amount(uint i) {
    return i & bitmask_surfel_amount;
}

bool is_child_octree_bit_set_at(uint i, uint pos)
{
    return (i & (1u << 31-pos)) != 0;
}

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec2 rand2(vec2 co) {
    return vec2(rand(co), rand(co + 1.23));// co + offset to get independent value
}

vec3 sample_hemisphere_uniform(vec2 uv) {
    uv = rand2(uv);
    float theta = acos(1 - uv.x);
    float phi = 2.0f *  PI * uv.y;
    return vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
}

//from https://iquilezles.org/articles/intersectors/
bool ray_surfel_intersection(Surfel s, Ray r, out vec3 hit_location)
{
    if (dot(s.normal.xyz, r.direction) > 0) return false;

    vec3  o = r.origin - s.mean_r.xyz;
    float t = -dot(s.normal.xyz,o)/dot(r.direction,s.normal.xyz);
    vec3  q = o + r.direction * t;
    hit_location = r.origin + r.direction*t;
    return (dot(q,q)<s.mean_r.w * s.mean_r.w) && (t >= 0 );
}


//adapted from https://iquilezles.org/articles/intersectors/
// axis aligned box centered at the origin, with size boxSize
bool boxIntersection(in Ray r, float boxSize, vec3 boxStartWS, out float distance, out float distanceNear)
{

    vec3 origin = r.origin - boxStartWS - boxSize * 0.5f; // transform the origin 
    vec3 n = r.inverse_direction * origin;   // can precompute if traversing a set of aligned boxes
    vec3 k = abs(r.inverse_direction)*boxSize * 0.5f;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = max( max( t1.x, t1.y ), t1.z );
    float tF = min( min( t2.x, t2.y ), t2.z );

    distanceNear = tN;
    distance = (tN < 0.0) ? 0.0 : tN;
    return  tN<tF && tF>0.0;
}

void insert_sorted(inout uint id_array[8], inout float distance_array[8], inout int count, uint new_id, float new_distance) {

    distance_array[7] = new_distance;
    id_array[7] = new_id;

    #pragma unroll
    for (int x = 7; x > 0 ; x--) {
        float value_right = distance_array[x];
        float value_left = distance_array[x-1];
        uint id_right = id_array[x];
        uint id_left = id_array[x-1];
        //right value should be larger than left one

        //float smaller = min(value_left,value_right);

        bool swap_values = value_left >= value_right; //repace if to reduce branching

        distance_array[x] = swap_values?value_left:distance_array[x];
        distance_array[x-1] = swap_values?value_right:distance_array[x-1];

        //also swap ids
        id_array[x] = swap_values?id_left:id_array[x];
        id_array[x - 1] = swap_values?id_right:id_array[x - 1];
    }
    count++;

}

#define MAX_VALUE 10000000.0f

//returns sorting order low -> high
int get_ordered_child_traversal(float extension_parent, vec3 parent_min, Ray r, out uint[8] ordered_ids){

    float child_size = extension_parent * 0.5f;
    int length_ids = 0;

    uint[8] id_array = uint[8](0u,0u,0u,0u,0u,0u,0u,0u);
    float[8] distance_array = float[8](MAX_VALUE,MAX_VALUE,MAX_VALUE,MAX_VALUE,MAX_VALUE,MAX_VALUE,MAX_VALUE,MAX_VALUE);

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
    uint id;
    float extension;
    vec3 min_pos;
};

bool traverseHERO(Ray ray, out vec3 c, out float d) {
    const int MAX_STACK = 64;
    stack_item stack[MAX_STACK];

    stack[0].id = 0;
    stack[0].extension = OCTREE_TOTOAL_EXTENSION;
    stack[0].min_pos = vec3(-OCTREE_TOTOAL_EXTENSION*0.5f);

    int stackPtr = 0;

    float closest_hit = MAX_VALUE;
    vec3 current_best_hit;
    bool has_hit = false;

    for (int tries = 0; tries < MAX_OCTREE_RAYTRACING_STEPS; tries++) {

        OctreeElement o = octreeElements[stack[stackPtr].id];
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
                    c = s.radiance_direct_and_surface.xyz + ((s.radiance_ambient.w > 100) ? s.radiance_ambient.xyz : vec3(0.0));
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
        int intersected_children = get_ordered_child_traversal(current_bucket_size, current_bucket_min, ray ,ordered_ids);
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
                stack[stackPtr].id = childIndex;
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

vec4 approx_lighting_for_pos(vec3 pos, float radius, vec3 normal, vec4 color_sampled_old) {
    vec3 color_out = vec3(0.0f);
    vec3 color_pre = color_sampled_old.rgb;
    float sampled_pre = color_sampled_old.a;
    for (int i = 0; i < ITERATIONS; i++) {
        vec2 random_seed = pos.xy*pos.z + i + sampled_pre + gl_LocalInvocationID.x;
        vec3 d = sample_hemisphere_uniform(random_seed);
        vec3 tangent;
        vec3 bitangent;


        getTangentBasis(normal, tangent, bitangent);

        vec3 s = normalize(normal * d.b + d.r * tangent + d.g * bitangent);
        s*= vec3(1.0);
        Ray r;
        r.origin = pos;
        r.direction = s;
        r.inverse_direction = 1.0f/s;
        r.max_length = 0.0f;
        
        vec3 c;
        float dist;
        if (!traverseHERO(r, c,dist)) {
            color_out+=get_ao_color(); //no object intersected -> skycolor
        } else {
            color_out+=c; //object intersected -> return object color;
        }
    }
    return vec4((color_out + color_pre * sampled_pre) / (ITERATIONS + sampled_pre)
    , sampled_pre + ITERATIONS);
}


uint get_next_octree_index_(uvec3 center, uvec3 pos)
{
    uint r = 0u;
    if (pos.x >= center.x)
    {
        r |= (1u << 2);
    }

    if (pos.y >= center.y)
    {
        r |= (1u << 1);
    }

    if (pos.z >= center.z)
    {
        r |= (1u << 0);
    }
    return r;
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

bool is_ws_pos_contained_in_bb(vec3 pos, vec3 bb_min, vec3 extension) {
    vec3 bb_max = bb_min + extension;
    return
    pos.x <= bb_max.x && pos.x >= bb_min.x &&
    pos.y <= bb_max.y && pos.y >= bb_min.y &&
    pos.z <= bb_max.z && pos.z >= bb_min.z;

}

//this method inserts the poitner of th updated octree node into the update array
void insert_update_info(uint p) {
    uint p_copy = p;
    uint free_spot = atomicAdd(allocationMetadata[0].octree_pointer_update_index, 1);
    atomicExchange(updatedIds[free_spot],p_copy);
}

void main() {
    uint p;
    vec3 metadata = vec3(0.0);

    const uint level = calculation_level;

    float node_size_at_level = OCTREE_TOTOAL_EXTENSION / float(1<<level);

    uvec3 center = pos_ws_start;
    vec3 bb_min = vec3(-OCTREE_TOTOAL_EXTENSION * 0.5f) + center * node_size_at_level;
    vec3 bb_extension = vec3(node_size_at_level);
    
    uint id = gl_GlobalInvocationID.x;

    if (get_surfe_pointer_at_octree_pos(level, center, p, metadata)){
        OctreeElement o = octreeElements[p];
        
        //insert this pointer into the list of changed surfels
        insert_update_info(p);
        
        uint surfels_amount = get_surfel_amount(o.surfels_at_layer_amount);
        if (id < surfels_amount) {
            uint surfle_data_pointer = o.surfels_at_layer_pointer;
            Surfel s = surfels[surfle_data_pointer + id];
            vec3 surfel_pos = s.mean_r.xyz;
            if (is_ws_pos_contained_in_bb(surfel_pos ,bb_min,bb_extension)) {
                //surfels[surfle_data_pointer + i].color = vec4(0,1,float(gl_LocalInvocationID.x == 0),1);
                vec4 final_ao_color = approx_lighting_for_pos(s.mean_r.xyz + s.normal.xyz * 0.1f, s.mean_r.w,s.normal.xyz, s.radiance_ambient);
                for (int i  = 0; i < 8; i++) {
                    uint location = s.copy_locations[i];
                    surfels[location].radiance_ambient = final_ao_color;
                }
            }
        }
    }
}
