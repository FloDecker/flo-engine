#version 430 core
#define PI 3.14159265359
#define OCTREE_TOTOAL_EXTENSION 512
#define OCTREE_HALF_TOTOAL_EXTENSION 256
#define BUCKET_SIZE 128
#define SURFELS_BUCKET_AMOUNT 80000
#define SURFEL_OCTREE_SIZE 100000
#define MAX_OCTREE_LEVEL 9
#define FOV 90

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

struct AllocationMetadata{
    uint surfel_bucket_pointer;
    uint surfel_octree_pointer;
    uint debug_int_32;
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

float bias = 0.01;
layout (std140,  binding = 1) uniform DIRECT_LIGHT_UNIFORMS
{
    vec3 direct_light_direction;
    float direct_light_intensity;
    vec3 direct_light_color;
    float direct_light_light_angle;
    mat4 direct_light_light_space_matrix;
};


layout (local_size_x = 8, local_size_y = 1, local_size_z = 1) in;
uniform sampler2D direct_light_map_texture;

uniform int offset_id;
uniform int calculation_level;

uniform sampler2D gPos;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSurfels;
uniform sampler2D gEmissive;

uniform vec3 camera_position;
uniform vec3 random_offset;

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

void getTangentBasis(vec3 normal, out vec3 tangent, out vec3 bitangent) {
    vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    tangent = normalize(cross(up, normal));
    bitangent = cross(normal, tangent);
}

uint get_next_octree_index_(uvec3 center, uvec3 pos)
{
    return (uint(pos.x >= center.x) << 2) |
    (uint(pos.y >= center.y) << 1) |
    (uint(pos.z >= center.z) << 0);
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
    pointer = atomicAdd(allocationMetadata[0].surfel_octree_pointer, 1);
    return (pointer < SURFEL_OCTREE_SIZE);
}

bool create_new_bucket(out uint pointer) {
    pointer = atomicAdd(allocationMetadata[0].surfel_bucket_pointer, BUCKET_SIZE);
    return pointer < BUCKET_SIZE * SURFELS_BUCKET_AMOUNT;
}


//LIGTMAPPING

float light_map_at(vec2 coords) {
    float a = texture(direct_light_map_texture, coords.xy).r;
    return a;
}



bool in_light_map_shadow(vec3 vertexPosWs) {
    vec4 frag_in_light_space = direct_light_light_space_matrix * vec4(vertexPosWs, 1.0);
    vec3 projCoords = frag_in_light_space.xyz / frag_in_light_space.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;
    float lm_depth = light_map_at(projCoords.xy);
    if (currentDepth - bias > lm_depth) {
        return true;
    }
    return false;
}



//returns the pointer to the surfels in octree node at pos x,y,z and level 
bool insert_surfel_at_octree_pos(Surfel s, uint level, uvec3 pos, out uint octree_node_index, out uint final_surfel_index) {
    uint current_element_index = 0;
    uvec3 last_min = uvec3(0);
    uint last_size = 1 << level;


    for (int i = 0; i < level; i ++) {
        last_size = last_size >> 1;// integer divison by 2
        uvec3 center = last_min + last_size;

        uint index = get_next_octree_index_(center,pos);
        
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
                    //set child octree bit
                    atomicOr(octreeElements[current_element_index].surfels_at_layer_amount, (1u << (31u - index)));
                    //set pointer to next node
                    atomicExchange(octreeElements[current_element_index].next_layer_surfels_pointer[index], p);
                } else {
                    return false;
                }
            } else {
                //TODO: implement this case
                
            }
        }
        
        // wait if needed
        do {
            cur = atomicCompSwap(octreeElements[current_element_index].next_layer_surfels_pointer[index],0u,0u);
        } while (cur == LOCK_SENTINAL || cur == 0u);
        
        current_element_index = cur;

        
        uvec3 offset = uvec3(
        (index & uint(1<<2)) != 0 ? 1 : 0,
        (index & uint(1<<1)) != 0 ? 1 : 0,
        (index & uint(1<<0)) != 0 ? 1 : 0
        );
        last_min += offset * last_size;
    }
    
    //insert if reached target level
    
    //check if a bucket exists
    uint cur = atomicCompSwap(octreeElements[current_element_index].surfels_at_layer_pointer, 0u, 0u);
    if (cur == 0u) {
        uint prev = atomicCompSwap(octreeElements[current_element_index].surfels_at_layer_pointer, 0u, LOCK_SENTINAL);
        //create bucket
        if (prev == 0u) {
            uint bucket_pointer;
            if (!create_new_bucket(bucket_pointer)) {
                atomicAdd(allocationMetadata[0].debug_int_32,1);

                return false;
            }
            
            memoryBarrierBuffer();
            atomicExchange(octreeElements[current_element_index].surfels_at_layer_pointer, bucket_pointer);
        } else {
            
        }
    }

    uint tries = 0;
    do {
        tries++;
        cur = atomicCompSwap(octreeElements[current_element_index].surfels_at_layer_pointer, 0u, 0u);

        if (tries > 10000){

            return false;
        }
    } while (cur == LOCK_SENTINAL || cur == 0u);
    
    

    if (get_surfel_amount(atomicCompSwap(octreeElements[current_element_index].surfels_at_layer_amount, 0u, 0u)) < BUCKET_SIZE){
        uint insert_at_local = atomicAdd(octreeElements[current_element_index].surfels_at_layer_amount, 1);
        uint insert_at_global = octreeElements[current_element_index].surfels_at_layer_pointer + get_surfel_amount(insert_at_local);
        octree_node_index = current_element_index;
        final_surfel_index = insert_at_global;
        memoryBarrierBuffer();
        surfels[insert_at_global] = s;
    } else {

        return false;
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

vec3 project_normal_onto_plane(vec3 v, vec3 plane_normal){
    return v- dot(v,plane_normal)* plane_normal;
}

vec4 get_average_color_of_bucket(uint octree_index_of_bucket) {
    vec4 avg = vec4(0.0);
    uint amount = get_surfel_amount(octreeElements[octree_index_of_bucket].surfels_at_layer_amount);
    uint index = octreeElements[octree_index_of_bucket].surfels_at_layer_pointer;
    for (uint i = 0; i < amount; i++){
        avg+=surfels[index + i].radiance_ambient;
    }
    return avg/float(amount);
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

shared uint[8] temp_copy_locations;

void main() {
    
    uvec2 sizeTex = textureSize(gNormal, 0);
    vec2 TexCoords = vec2(gl_WorkGroupID.xy) / sizeTex;
    
    const float edge_distance = 0.05;
    
    if(TexCoords.x < edge_distance) {
        return;
    }

    if(TexCoords.y < edge_distance) {
        return;
    }

    if(TexCoords.x > 1-edge_distance) {
        return;
    }

    if(TexCoords.y > 1-edge_distance) {
        return;
    }




    vec3 normal_ws = vec3(texture(gNormal, TexCoords));
    vec3 pos_ws = vec3(texture(gPos, TexCoords));
    vec3 albedo = vec3(texture(gAlbedo, TexCoords));
    vec3 emissive = vec3(texture(gEmissive, TexCoords));
    vec4 surfel_buffer = vec4(texture(gSurfels, TexCoords));

    if (!is_ws_pos_contained_in_bb(pos_ws, vec3(-OCTREE_HALF_TOTOAL_EXTENSION), vec3(OCTREE_TOTOAL_EXTENSION))) {
        return;
    }


    float d_camera_pos = distance(camera_position,pos_ws);
    vec3 view_direction = normalize(pos_ws - camera_position);


    float target_distance = 5.0;
    float target_radius_pixels = 128.0;//real_world_size.x;
    float fov_rad = FOV * PI / 180.0;
    

    
    float ws_radius_min = 1.0f;
    

    //TODO: could be replaced by the projection matrix
    
    vec2 real_world_size = 2.0 * d_camera_pos * tan(fov_rad*0.5f) * (target_radius_pixels/sizeTex.xy);

    real_world_size = max(vec2(ws_radius_min), real_world_size);
    
    target_radius_pixels = ((real_world_size * sizeTex.xy) / (2.0 * d_camera_pos * tan(fov_rad*0.5f))).x;
    
    
    uvec2 required_pixel_interval = uvec2(target_radius_pixels);

    if (surfel_buffer.a > 0.1) {
        return;
    }
    
    uvec2 random_pos_in_intervall = uvec2(vec2(required_pixel_interval) * random_offset.xy);
    
    if(mod(gl_WorkGroupID.xy, required_pixel_interval) != uvec2(random_pos_in_intervall) ) {
        return;
    };
    
    
    float radius = real_world_size.x;

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
    s.radiance_ambient = vec4(0.2,0.2,0.2,0.0);

    float NdotL = dot(normal_ws, normalize(direct_light_direction));
    float diffuseIntensity = clamp(NdotL,0,1);
    
    vec3 direct_light = (in_light_map_shadow(pos_ws) ? vec3(0.0) : 
    direct_light_intensity * direct_light_color * albedo * diffuseIntensity);
    
    
    s.radiance_direct_and_surface = vec4(direct_light + emissive,1.0);
    s.normal = vec4(normal_ws,0);
    
    
    
    uint octree_index_of_bucket;
    uint final_surfel_index;
    if (insert_surfel_at_octree_pos(s, level, cell_index + offset_vector * component_multiplier[gl_LocalInvocationID.x],octree_index_of_bucket, final_surfel_index)){
        atomicAdd(allocationMetadata[0].debug_int_32,1);
    }
    temp_copy_locations[gl_LocalInvocationID.x] = final_surfel_index;
    barrier();
    
    for (int i = 0; i < 8; i++) {
        surfels[final_surfel_index].copy_locations[i] = temp_copy_locations[i];
    }

}
