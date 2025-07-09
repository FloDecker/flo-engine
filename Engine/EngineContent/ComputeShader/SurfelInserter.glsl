#version 430 core
#define PI 3.14159265359
#define OCTREE_TOTOAL_EXTENSION 512
#define OCTREE_HALF_TOTOAL_EXTENSION 256
#define BUCKET_SIZE 32
#define MAX_OCTREE_LEVEL 9

struct Surfel {
    vec4 mean_r;
    vec4 normal;
    vec4 color;
};

struct OctreeElement
{
    uint surfels_at_layer_amount;
    uint surfels_at_layer_pointer;
    uint next_layer_surfels_pointer[8];
};

struct AllocationMetadata{
    uint surfel_bucket_pointer;
    uint surfel_octree_pointer;
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

layout(std430, binding = 2) buffer AllocationMetadataBuffer {
    AllocationMetadata allocationMetadata[];
};

layout (local_size_x = 8, local_size_y = 1, local_size_z = 1) in;

uniform int offset_id;
uniform int calculation_level;

uniform sampler2D gPos;
uniform sampler2D gNormal;

const uint LOCK_SENTINAL = 0xFFFFFFFFu;

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
            c = vec3(0, 0, 1);
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
                    c = vec3(tries/1000.0f);
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

vec4 approx_lighting_for_pos(vec3 pos, vec3 normal, vec4 color_sampled_old) {
    vec3 color_out = vec3(0.0f);
    vec3 color_pre = color_sampled_old.rgb;
    float sampled_pre = color_sampled_old.a;
    const int iterations = 2;
    for (int i = 0; i < iterations; i++) {
        vec3 d =sampleHemisphereUniform(pos.xy*pos.z + i + sampled_pre + gl_LocalInvocationID.y);
        vec3 tangent;
        vec3 bitangent;

        getTangentBasis(normal, tangent, bitangent);
        vec3 s = normalize(normal * d.b + d.r * tangent + d.g * bitangent);
        s*= vec3(1.0);
        Ray r;
        r.origin = pos;
        r.direction = s;
        r.inverse_direction = 1.0f/s;

        vec3 c;
        if (!traverseHERO(r, c)) {
            color_out+=vec3(1.0);
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

uvec3 get_cell_size_at_level(uint level) {
    uint buckets = 1 << level;
    return uvec3(vec3(OCTREE_TOTOAL_EXTENSION) / buckets);
    
}

uvec3 get_surfel_cell_index_from_ws(vec3 ws, uint level) {
    ws+=vec3(OCTREE_HALF_TOTOAL_EXTENSION);
    return uvec3(ws / get_cell_size_at_level(level));
}

bool create_new_surfel_node(out uint pointer) {
    //TODO: check if out of ssbo bounds
    pointer = atomicAdd(allocationMetadata[0].surfel_octree_pointer, 1);
    return true;
}

bool create_new_bucket(out uint pointer) {
    //TODO: check if out of ssbo bounds
    pointer = atomicAdd(allocationMetadata[0].surfel_bucket_pointer, BUCKET_SIZE);
    return true;
}

uint set_child_octree_bit_at(uint amount_field, uint pos)
{
    return atomicOr(amount_field, (1u << (31u - pos)));
}

//returns the pointer to the surfels in octree node at pos x,y,z and level 
bool insert_surfel_at_octree_pos(Surfel s, uint level, uvec3 pos) {
    uint current_element_index = 0;
    uvec3 last_min = uvec3(0);
    uint last_size = 1 << level;


    for (int i = 0; i < level; i ++) {
        last_size = last_size >> 1;// integer divison by 2
        uvec3 center = last_min + last_size;

        uint index = get_pos_of_next_surfel_index_(center, pos);
        
        //read the current value of surfels_at_layer_amount
        //atomic read -> if mem != 0u dont exchange, if mem == 0u its replaced by 0u -> idempotent
        uint cur = atomicCompSwap(octreeElements[current_element_index].next_layer_surfels_pointer[index], 0u, 0u);
        if (cur == 0u) {
            //node has to be created
            //try to aquire lock
            uint prev = atomicCompSwap(octreeElements[current_element_index].next_layer_surfels_pointer[index], 0u, LOCK_SENTINAL);
            if (prev == 0u) {
                uint p;
                if (create_new_surfel_node(p)) {
                    
                    memoryBarrierBuffer();
                    barrier();
                    
                    //set child octree bit
                    atomicOr(octreeElements[current_element_index].surfels_at_layer_amount, (1u << (31u - index)));
                    //set pointer to next node
                    atomicExchange(octreeElements[current_element_index].next_layer_surfels_pointer[index], p);
                }
            } else {
                //TODO: implement this case
            }
        }
        
        // wait if needed
        do {
            cur = atomicCompSwap(octreeElements[current_element_index].next_layer_surfels_pointer[index],0u,0u);
        } while (cur == LOCK_SENTINAL || cur == 0u);
        
        current_element_index = octreeElements[current_element_index].next_layer_surfels_pointer[index];

        
        uvec3 offset = uvec3(
        (index & uint(1<<2)) != 0 ? 1 : 0,
        (index & uint(1<<1)) != 0 ? 1 : 0,
        (index & uint(1<<0)) != 0 ? 1 : 0
        );
        last_min += offset * last_size;
    }
    
    //insert if reached target level
    
    //check if a bucket exists
    if (get_surfel_amount(octreeElements[current_element_index].surfels_at_layer_amount) == 0) {
        uint prev = atomicCompSwap(octreeElements[current_element_index].surfels_at_layer_pointer, 0u, LOCK_SENTINAL);
        //create bucket
        if (prev == 0u) {
            uint bucket_pointer;
            if (!create_new_bucket(bucket_pointer)) {
                //    return false;
            }
            memoryBarrierBuffer();
            barrier();
            atomicExchange(octreeElements[current_element_index].surfels_at_layer_pointer, bucket_pointer);
        } else {
            while (prev == LOCK_SENTINAL) {
                prev = atomicCompSwap(octreeElements[current_element_index].surfels_at_layer_pointer, 0u, 0u);
            }
        }
        
    }

    if (get_surfel_amount(octreeElements[current_element_index].surfels_at_layer_amount) < BUCKET_SIZE){
        uint insert_at_local = atomicAdd(octreeElements[current_element_index].surfels_at_layer_amount, 1);
        uint insert_at_global = octreeElements[current_element_index].surfels_at_layer_pointer + insert_at_local;
        memoryBarrierBuffer();
        surfels[insert_at_global] = s;
    }
    return true;
}

bool is_ws_pos_contained_in_bb(vec3 pos, vec3 bb_min, vec3 extension) {
    vec3 bb_max = bb_min + extension;
    return
    pos.x <= bb_max.x && pos.x >= bb_min.x &&
    pos.y <= bb_max.y && pos.y >= bb_min.y &&
    pos.z <= bb_max.z && pos.z >= bb_min.z;

}


uint get_octree_level_for_surfel(float radius)
{
float d = radius * 2.0f;
int level_surfel = int(ceil(log2(d)));
int level_bounds = int(ceil(log2(OCTREE_TOTOAL_EXTENSION)));
return min(level_bounds - level_surfel, MAX_OCTREE_LEVEL);
}

const uvec3 component_multiplier[8] = {
uvec3(1, 1, 1),
uvec3(1, 1, 0),
uvec3(1, 0, 1),
uvec3(1, 0, 0),
uvec3(0, 1, 1),
uvec3(0, 1, 0),
uvec3(0, 0, 1),
uvec3(0, 0, 0),
};

void main() {
    float radius = 0.2;
    
    vec2 TexCoords = vec2(gl_WorkGroupID.xy) / vec2(64);
    vec3 normal_ws = vec3(texture(gNormal, TexCoords));
    vec3 pos_ws = vec3(texture(gPos, TexCoords));
    
    
    
    uint level = get_octree_level_for_surfel(radius);
    
    //calculate overlap
    uvec3 cell_index = get_surfel_cell_index_from_ws(pos_ws, level);
    
    //local offset inside cell
    vec3 cell_size = get_cell_size_at_level(level);
    vec3 offset_from_cell_center = mod(pos_ws, cell_size) / cell_size;
    
    uvec3 offset_vector = uvec3(0);
    offset_vector.x = offset_from_cell_center.x < 0.5f ? -1 : 1;
    offset_vector.y = offset_from_cell_center.y < 0.5f ? -1 : 1;
    offset_vector.z = offset_from_cell_center.z < 0.5f ? -1 : 1;
    
    
    
    Surfel s;
    s.mean_r = vec4(pos_ws, radius);
    s.color = vec4(1.0);
    s.normal = vec4(normal_ws,0);
    
    //octreeElements[0].surfels_at_layer_amount = get_pos_of_next_surfel_index_(uvec3(256,256,256), pos);
    
    
    insert_surfel_at_octree_pos(s, level, cell_index + offset_vector * component_multiplier[gl_LocalInvocationID.x]);
}
