#version 430 core
#define PI 3.14159265359
#define OCTREE_TOTOAL_EXTENSION 512
#define OCTREE_HALF_TOTOAL_EXTENSION 256
#define BUCKET_SIZE 256
#define SURFELS_BUCKET_AMOUNT 40000
#define SURFEL_OCTREE_SIZE 100000
#define MAX_OCTREE_LEVEL 9
#define FOV 90
#define MAX_TRIES_INSERTION 10000
#define BITMASK_SURFEL_AMOUNT 0x00FFFFFFu
#define BIAS_SHADOW_MAPPING 0.01f
#define LOCK_SENTINEL 0xFFFFFFFFu

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

struct AllocationMetadata{
    uint surfel_bucket_pointer;
    uint surfel_octree_pointer;
    uint octree_pointer_update_index;
    uint debug_int_32;
};

struct Ray {
    vec3 origin;
    vec3 direction;
    vec3 inverse_direction;
};

layout (local_size_x = 8, local_size_y = 1, local_size_z = 1) in;

//SURFEL BACKBUFFER
layout(std430, binding = 3) buffer SurfelBuffer {
    Surfel surfels[];
};
//OCTREE BACKBUFFER
layout(std430, binding = 5) buffer OctreeBuffer {
    OctreeElement octreeElements[];
};

layout(std430, binding = 2) buffer AllocationMetadataBuffer {
    AllocationMetadata allocationMetadata[];
};

layout(std430, binding = 8) buffer UpdatedOctreeElements {
    uint updatedIds[];
};

layout (std140, binding = 1) uniform DIRECT_LIGHT_UNIFORMS
{
    vec3 direct_light_direction;
    float direct_light_intensity;
    vec3 direct_light_color;
    float direct_light_light_angle;
    mat4 direct_light_light_space_matrix;
};


uniform sampler2D direct_light_map_texture;

uniform int offset_id;
uniform int calculation_level;

uniform float minimal_surfel_radius;
uniform float surfel_insertion_threshold;
uniform float surfel_insert_size_multiplier;
uniform int pixels_per_surfel;
uniform vec3 camera_position;
uniform vec3 random_offset;

uniform sampler2D gPos;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSurfels;
uniform sampler2D gEmissive;
uniform sampler2D surfel_framebuffer_metadata_0;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;

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
    return i & BITMASK_SURFEL_AMOUNT;
}

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
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

//this method inserts the poitner of th updated octree node into the update array
void insert_update_info(uint p) {
    uint p_copy = p;
    uint free_spot = atomicAdd(allocationMetadata[0].octree_pointer_update_index, 1);
    atomicExchange(updatedIds[free_spot], p_copy);
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
    return (currentDepth - BIAS_SHADOW_MAPPING > lm_depth);
}

//returns the pointer to the surfels in octree node at pos x,y,z and level 
bool insert_surfel_at_octree_pos(Surfel s, uint level, uvec3 pos, out uint octree_node_index, out uint final_surfel_index) {
    uint current_element_index = 0;
    uvec3 last_min = uvec3(0);
    uint last_size = 1 << level;

    for (int i = 0; i < level; i ++) {
        last_size = last_size >> 1;// integer divison by 2
        uvec3 center = last_min + last_size;
        uint index = get_next_octree_index_(center, pos);

        //atomic read -> if mem != 0u dont exchange, if mem == 0u its replaced by 0u -> idempotent
        uint cur = atomicCompSwap(octreeElements[current_element_index].next_layer_surfels_pointer[index], 0u, 0u);
        if (cur == 0u) {
            //node has to be created
            //try to aquire lock
            uint prev = atomicCompSwap(octreeElements[current_element_index].next_layer_surfels_pointer[index], 0u, LOCK_SENTINEL);
            if (prev == 0u) {
                uint p;
                if (create_new_surfel_node(p)) {
                    //set child octree bit
                    atomicOr(octreeElements[current_element_index].surfels_at_layer_amount, (1u << (31u - index)));
                    //set pointer to next node
                    atomicExchange(octreeElements[current_element_index].next_layer_surfels_pointer[index], p);
                    insert_update_info(current_element_index);
                    insert_update_info(p);
                    memoryBarrierBuffer();
                } else {
                    return false;
                }
            }
        }

        uint tries = 0;
        do {
            cur = atomicCompSwap(octreeElements[current_element_index].next_layer_surfels_pointer[index], 0u, 0u);
            tries++;
            if (tries > MAX_TRIES_INSERTION){
                return false;
            }
        } while (cur == LOCK_SENTINEL);

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
        uint prev = atomicCompSwap(octreeElements[current_element_index].surfels_at_layer_pointer, 0u, LOCK_SENTINEL);
        //create bucket
        if (prev == 0u) {
            uint bucket_pointer;
            if (!create_new_bucket(bucket_pointer)) {
                return false;
            }
            atomicExchange(octreeElements[current_element_index].surfels_at_layer_pointer, bucket_pointer);
            memoryBarrierBuffer();
        }
    }

    uint tries = 0;
    do {
        cur = atomicCompSwap(octreeElements[current_element_index].surfels_at_layer_pointer, 0u, 0u);
        tries++;
        if (tries > MAX_TRIES_INSERTION){
            return false;
        }
    } while (cur == LOCK_SENTINEL);

    if (get_surfel_amount(atomicCompSwap(octreeElements[current_element_index].surfels_at_layer_amount, 0u, 0u)) < BUCKET_SIZE){
        uint insert_at_local = atomicAdd(octreeElements[current_element_index].surfels_at_layer_amount, 1);
        uint insert_at_global = octreeElements[current_element_index].surfels_at_layer_pointer + get_surfel_amount(insert_at_local);
        octree_node_index = current_element_index;
        final_surfel_index = insert_at_global;
        memoryBarrierBuffer();
        surfels[insert_at_global] = s;

        insert_update_info(current_element_index);

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
    float pixels_per_surfel_f = float(pixels_per_surfel);
    uvec2 sizeTex = textureSize(gNormal, 0);
    vec2 TexCoords_original = (vec2(gl_WorkGroupID.xy * pixels_per_surfel_f)) / sizeTex;
    TexCoords_original+=random_offset.xy * (pixels_per_surfel_f / sizeTex);
    vec3 pos_ws_original = vec3(texture(gPos, TexCoords_original));


    float d_camera_pos = -(view_matrix * vec4(pos_ws_original, 1.0)).z;//distance(camera_position,pos_ws_original);

    //FOV is defined in the y direction 
    float fov_rad = FOV * PI / 180.0;

    float height_camera = 2.0 * tan(fov_rad*0.5f);
    float surfel_radius_on_camera_plane = height_camera / ((sizeTex.y/ pixels_per_surfel_f));

    float real_world_diameter = (d_camera_pos) * surfel_radius_on_camera_plane;

    real_world_diameter = max(minimal_surfel_radius * 2.0, real_world_diameter);

    float acutal_pixel_diameter = (real_world_diameter / (d_camera_pos));

    vec2 ndc = TexCoords_original * 2.0 - 1.0f;
    ndc*=acutal_pixel_diameter/surfel_radius_on_camera_plane;

    vec2 TexCoords = (ndc.rg + 1.0) * 0.5f;

    const float edge_distance = 0.01;

    if (TexCoords.y < edge_distance ||
    TexCoords.x < edge_distance ||
    TexCoords.x > 1-edge_distance
    || TexCoords.y > 1-edge_distance) {
        return;
    }




    vec3 normal_ws = vec3(texture(gNormal, TexCoords));
    vec3 pos_ws = vec3(texture(gPos, TexCoords));
    vec3 albedo = vec3(texture(gAlbedo, TexCoords));
    vec3 emissive = vec3(texture(gEmissive, TexCoords));
    vec4 surfel_buffer = vec4(texture(gSurfels, TexCoords));

    if (!is_ws_pos_contained_in_bb(pos_ws, vec3(-OCTREE_HALF_TOTOAL_EXTENSION), vec3(OCTREE_TOTOAL_EXTENSION)) ||
    surfel_buffer.a > surfel_insertion_threshold) {
        return;
    }



    float radius = (real_world_diameter * 0.5f)*surfel_insert_size_multiplier;

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

    vec3 surfel_metadata_0 = vec3(texture(surfel_framebuffer_metadata_0, TexCoords));


    Surfel s;
    s.mean_r = vec4(pos_ws, radius);
    s.radiance_ambient = vec4(surfel_metadata_0.rgb, 16.0);
    s.albedo = vec4(albedo, 0.0);

    float NdotL = dot(normal_ws, normalize(direct_light_direction));
    float diffuseIntensity = clamp(NdotL, 0, 1);

    vec3 direct_light = (in_light_map_shadow(pos_ws) ? vec3(0.0) :
    direct_light_intensity * direct_light_color * albedo * diffuseIntensity);


    s.radiance_direct_and_surface = vec4(direct_light + emissive, 1.0);
    s.normal = vec4(normal_ws, 0);



    uint octree_index_of_bucket;
    uint final_surfel_index;
    if (insert_surfel_at_octree_pos(s, level, cell_index + offset_vector * component_multiplier[gl_LocalInvocationID.x], octree_index_of_bucket, final_surfel_index)){
        atomicAdd(allocationMetadata[0].debug_int_32, 1);
    }
    temp_copy_locations[gl_LocalInvocationID.x] = final_surfel_index;
    barrier();

    for (int i = 0; i < 8; i++) {
        surfels[final_surfel_index].copy_locations[i] = temp_copy_locations[i];
    }

}
