#version 430 core
#define BITMASK_SURFEL_AMOUNT 0x00FFFFFFu

/*
Computeshader copys the data that has changed from the frontbuffer to the backbuffer
Corresponds to section 4.6 in the thesis
*/

//STRUCT DEFINITIONS
struct Surfel {
    vec4 mean_r;
    vec4 normal;
    vec4 albedo;
    vec4 radiance_ambient; //radiance without surface irradiance and direct light 
    vec4 radiance_direct_and_surface; //radiance contribution from direct light and surface
    uint[8] copy_locations; //global adresses where this exact surfel can be found
};

struct OctreeElement
{
    uint surfels_at_layer_amount;
    uint surfel_bucket_pointer;
    uint child_nodes_pointer[8];
};

struct AllocationMetadata{
    uint surfel_bucket_pointer;
    uint surfel_octree_pointer;
    uint octree_pointer_update_index;
    uint debug_int_32;
};

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// BACKBUFFERS
layout(std430, binding = 3) buffer SurfelBufferBack {
    Surfel surfels_back[];
};

layout(std430, binding = 5) buffer OctreeBufferBack {
    OctreeElement octreeElements_back[];
};

// FRONT BUFFERS
layout(std430, binding = 4) readonly buffer SurfelBufferFront {
    Surfel surfels_front[];
};

layout(std430, binding = 6) readonly buffer OctreeBufferFront {
    OctreeElement octreeElements_front[];
};

// CHANGED POINTERS IN LAST SURFEL UPDATE LOOP
layout(std430, binding = 8) buffer UpdatedOctreeElements {
    uint updatedIds[];
};

//OCTREE METADATA
layout(std430, binding = 2) buffer AllocationMetadataBuffer {
    AllocationMetadata allocationMetadata[];
};

uint get_surfel_amount(uint i) {
    return i & BITMASK_SURFEL_AMOUNT;
}

void main() {
    uint id_in_update_array = gl_WorkGroupID.x;
    uint pointer_to_update = updatedIds[id_in_update_array];
    
    OctreeElement octree_front = octreeElements_front[pointer_to_update];
    
    //update octree element
    octreeElements_back[pointer_to_update] = octree_front;
    
    //update surfels of octree element
    uint surfel_pointer = octree_front.surfel_bucket_pointer;
    for (uint i = 0; i < get_surfel_amount(octree_front.surfels_at_layer_amount); i++) {
        Surfel surfel_front = surfels_front[surfel_pointer + i];
        surfels_back[surfel_pointer + i] = surfel_front;
        for (uint j = 0; j < 8; j++) {
            uint copy_location_pointer = surfel_front.copy_locations[j];
            surfels_back[copy_location_pointer] = surfel_front;
        }
    }
    
    //clear update buffer
    updatedIds[id_in_update_array] = 0;
    
    atomicExchange(allocationMetadata[0].octree_pointer_update_index,0);
}
