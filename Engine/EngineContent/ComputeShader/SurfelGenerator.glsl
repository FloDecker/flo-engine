#version 430 core

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

layout(std430, binding = 0) buffer SurfelBuffer {
    Surfel surfels[];
};

layout(std430, binding = 1) buffer OctreeBuffer {
    OctreeElement octreeElements[];
};

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main() {
    
}
