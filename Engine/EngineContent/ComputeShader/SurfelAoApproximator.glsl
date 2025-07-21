#version 430 core
#define PI 3.14159265359
#define OCTREE_TOTOAL_EXTENSION 512

struct Surfel {
    vec4 mean_r;
    vec4 normal;
    vec4 radiance_ambient; //radiance without surface irradiance and direct light 
    vec4 radiance_direct_and_surface; //radiance contribution from direct light and surface
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
};

layout(std430, binding = 0) buffer SurfelBuffer {
    Surfel surfels[];
};

layout(std430, binding = 1) buffer OctreeBuffer {
    OctreeElement octreeElements[];
};

layout (local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

uniform int offset_id;
uniform int calculation_level;
uniform uvec3 pos_ws_start;


uint bitmask_surfel_amount = 0x00FFFFFF;

bool intersect_AABB(vec3 bb_min, vec3 bb_max, Ray ray) {
    vec3 t1 = (bb_min - ray.origin) * ray.inverse_direction;
    vec3 t2 = (bb_max - ray.origin) * ray.inverse_direction;
    vec3 tmin3 = min(t1, t2);
    vec3 tmax3 = max(t1, t2);
    float tmin = max(max(tmin3.x, tmin3.y), tmin3.z);
    float tmax = min(min(tmax3.x, tmax3.y), tmax3.z);
    return tmax >= tmin;
}

uint get_surfel_amount(uint i) {
    return i & bitmask_surfel_amount;
}

bool is_child_octree_bit_set_at(uint i, uint pos)
{
    return (i & (1u << 31-pos)) != 0;
}

bool are_all_child_octree_bits_empty(uint i)
{
    return (bitmask_surfel_amount & i) == 0;
}

bool ray_surfel_intersection(Surfel s, Ray r, out vec3 hit_location) {
    if (dot(s.normal.xyz, r.direction) > 0) return false;
    float d = -dot(s.normal.xyz, s.mean_r.xyz);
    float t = -((dot(s.normal.xyz, r.origin) + d) /
    dot(s.normal.xyz, r.direction));
    if (t <= 0) return false;
    hit_location = r.origin + t * r.direction;
    return distance(s.mean_r.xyz, hit_location) <= s.mean_r.w;
}

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec2 rand2(vec2 co) {
    return vec2(rand(co), rand(co + 1.23));// co + offset to get independent value
}

vec2 sample_disk(vec2 u) {
    // Map uniform random point u from [0,1]^2 to [-1,1]^2
    float sx = 2.0 * u.x - 1.0;
    float sy = 2.0 * u.y - 1.0;

    // Handle the degeneracy at the origin
    if (sx == 0.0 && sy == 0.0) {
        return vec2(0.0);
    }

    float r, theta;

    // Concentric mapping (Shirley-Chiu 1997)
    if (abs(sx) > abs(sy)) {
        r = sx;
        theta = (PI / 4.0) * (sy / sx);
    } else {
        r = sy;
        theta = (PI / 2.0) - (PI / 4.0) * (sx / sy);
    }

    return r * vec2(cos(theta), sin(theta));
}

vec3 sampleHemisphereUniform(vec2 uv) {
    uv = rand2(uv);
    float theta = acos(1 - uv.x);
    float phi = 2.0f *  PI * uv.y;
    return vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
}


bool traverseHERO(Ray ray, out vec3 c) {
    const int MAX_DEPTH = 8;
    const int MAX_STACK = 64;

    // Stack to simulate traversal
    uint node_ids_stack[MAX_STACK];
    float node_size_stack[MAX_STACK];
    vec3 node_min_stack[MAX_STACK];
    node_ids_stack[0] = 0;
    node_size_stack[0] = OCTREE_TOTOAL_EXTENSION;
    node_min_stack[0] = vec3(-OCTREE_TOTOAL_EXTENSION*0.5f);

    int stackPtr = 0;

    float closest_hit = 10000000;
    vec3 current_best_hit;
    bool has_hit = false;

    for (int tries = 0; tries < 100; tries++) {
        OctreeElement o = octreeElements[node_ids_stack[stackPtr]];
        float current_bucket_size = node_size_stack[stackPtr];
        vec3 current_bucket_min = node_min_stack[stackPtr];
        if (stackPtr < 0) {
            c = vec3(0, 0, 0);
            return false;
        }

        //check if there are surfels hit on the current layer 
        uint surfels_amount = get_surfel_amount(o.surfels_at_layer_amount);
        if (surfels_amount > 0) {
            uint surfle_data_pointer = o.surfels_at_layer_pointer;
            for (int i = 0; i < surfels_amount; i++) {
                Surfel s = surfels[surfle_data_pointer + i];
                vec3 hit_location;
                if (ray_surfel_intersection(s, ray, hit_location)) {
                    has_hit =true;//remvoe
                    c = s.radiance_direct_and_surface.xyz;
                    return true;
                    if (distance(hit_location, ray.origin) < closest_hit) {
                        closest_hit = distance(hit_location, ray.origin);
                        current_best_hit = hit_location;
                        has_hit =true;
                    }
                }
            }
        }
        stackPtr--;

        //if there was an intersection and the current octree node doesent have children -> return
        //if (are_all_child_octree_bits_empty(o.surfels_at_layer_amount) && has_hit)  {
        //    return true;
        //}


        //if there are no intersections 


        int xDir = ray.direction.x >= 0.0 ? 0 : 1;
        int yDir = ray.direction.y >= 0.0 ? 0 : 1;
        int zDir = ray.direction.z >= 0.0 ? 0 : 1;


        float children_size = current_bucket_size * 0.5f;

        for (int i = 0; i < 8; ++i) {
            int morton = i;//^ (zDir | (yDir << 1) | (xDir << 2));
            if (!is_child_octree_bit_set_at(o.surfels_at_layer_amount, morton)) continue;
            uint childIndex = o.next_layer_surfels_pointer[morton];

            vec3 offset = vec3(
            (morton & (1<<2)) != 0 ? 1.0 : 0.0,
            (morton & (1<<1)) != 0 ? 1.0 : 0.0,
            (morton & (1<<0)) != 0 ? 1.0 : 0.0
            );


            // Descend into child
            vec3 child_min = current_bucket_min + offset * children_size;

            if (!intersect_AABB(child_min, child_min + vec3(children_size), ray)) continue;


            // Push remaining children to stack
            if (stackPtr < MAX_STACK) {
                stackPtr++;
                node_ids_stack[stackPtr] = childIndex;
                node_size_stack[stackPtr] = children_size;
                node_min_stack[stackPtr] = child_min;
            }
        }
    }

    return false;
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
    const int iterations = 5;
    for (int i = 0; i < iterations; i++) {
        vec2 random_seed = pos.xy*pos.z + i + sampled_pre + gl_LocalInvocationID.y;
        vec3 d =sampleHemisphereUniform(random_seed);
        vec3 tangent;
        vec3 bitangent;

        
        getTangentBasis(normal, tangent, bitangent);

        vec2 offset = sample_disk(rand2(random_seed)) * radius;
        vec3 s = normalize(normal * d.b + d.r * tangent + d.g * bitangent);
        s*= vec3(1.0);
        Ray r;
        r.origin = pos; //+ offset.x * tangent + offset.y * bitangent;
        r.direction = s;
        r.inverse_direction = 1.0f/s;

        vec3 c;
        if (!traverseHERO(r, c)) {
            color_out+=vec3(1.0);
        } else {
            color_out+=c;
        }
    }
    return vec4((color_out + color_pre * sampled_pre) / (iterations + sampled_pre)
    , sampled_pre + iterations);
}



uint get_pos_of_next_surfel_index_(uvec3 center, uvec3 pos)
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

        uint index = get_pos_of_next_surfel_index_(center, pos);

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

void calculate_light_for_surfels_in_bucket(uint bucket_index, uint surfels_amount, vec3 bb_min, vec3 bb_extension) {

    for (int i = 0; i < surfels_amount; i++) { 
        Surfel s = surfels[bucket_index + i];
        
        if (is_ws_pos_contained_in_bb(s.mean_r.xyz, bb_min, bb_extension)) {
            surfels[bucket_index + i].radiance_ambient = vec4(0,1,1,0);
        }

    }

}

const uvec3 pos_offset_3x3[27] = {

uvec3(0, 0, 0),


uvec3(0, 1, 1),
uvec3(0, 1, 0),
uvec3(0, 1, -1),
uvec3(0, 0, 1),
uvec3(0, 0, -1),
uvec3(0, -1, 1),
uvec3(0, -1, 0),
uvec3(0, -1, -1),

uvec3(1, 1, 1),
uvec3(1, 1, 0),
uvec3(1, 1, -1),
uvec3(1, 0, 1),
uvec3(1, 0, 0),
uvec3(1, 0, -1),
uvec3(1, -1, 1),
uvec3(1, -1, 0),
uvec3(1, -1, -1),

uvec3(-1, 1, 1),
uvec3(-1, 1, 0),
uvec3(-1, 1, -1),
uvec3(-1, 0, 1),
uvec3(-1, 0, 0),
uvec3(-1, 0, -1),
uvec3(-1, -1, 1),
uvec3(-1, -1, 0),
uvec3(-1, -1, -1),
};

void main() {

    uint p;
    vec3 metadata = vec3(0.0);
    
    const uint level = calculation_level;

    float node_size_at_level = OCTREE_TOTOAL_EXTENSION / float(1<<level);
    
    uint flatIndex = gl_WorkGroupID.x +
    gl_NumWorkGroups.x * (gl_WorkGroupID.y +
    gl_NumWorkGroups.y * gl_WorkGroupID.z);

    uvec3 center = pos_ws_start;
    vec3 bb_min = vec3(-OCTREE_TOTOAL_EXTENSION * 0.5f) + center * node_size_at_level;
    vec3 bb_extension = vec3(node_size_at_level);

    uvec3 offset_center = center +  pos_offset_3x3[flatIndex] ;
    if (get_surfe_pointer_at_octree_pos(level, offset_center, p, metadata)){
        OctreeElement o = octreeElements[p];

        uint surfels_amount = get_surfel_amount(o.surfels_at_layer_amount);
        if (gl_LocalInvocationID.x < surfels_amount) {
            uint surfle_data_pointer = o.surfels_at_layer_pointer;
            Surfel s = surfels[surfle_data_pointer + gl_LocalInvocationID.x];
            vec3 surfel_pos = s.mean_r.xyz;
            if (is_ws_pos_contained_in_bb(surfel_pos ,bb_min,bb_extension)) {
                //surfels[surfle_data_pointer + i].color = vec4(0,1,float(gl_LocalInvocationID.x == 0),1);
                surfels[surfle_data_pointer + gl_LocalInvocationID.x].radiance_ambient = approx_lighting_for_pos(s.mean_r.xyz + s.normal.xyz * 0.1f, s.mean_r.w,s.normal.xyz, s.radiance_ambient);
            }
        }
    }
}
