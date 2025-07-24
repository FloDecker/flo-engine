#version 430 core

#define MAX_VALUE 50000000
#define MAX_SAMPLES_PER_SURFEL 512
layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

uniform sampler2D surfel_framebuffer_metadata;
uniform sampler2D gPos;

layout(std430, binding = 7) buffer LeastShaded {
    vec4 least_shaded_regions[];
};



shared int min_value;

shared uint flatted_index_of_best_pixel;
void main() {
    
    //thread 0,0  initalizes varaibles 
    if (gl_LocalInvocationID.xy != uvec2(0)) {
        min_value  = MAX_VALUE;
        flatted_index_of_best_pixel = 0u;
    }
    barrier();
    

    uvec2 size_texutre = uvec2(textureSize(surfel_framebuffer_metadata,0).xy);
    
    float scaling_factor = 1.0f;
    //4 = scaling factor 
    vec2 single_pixel_offset = 1.0f/vec2(size_texutre) * scaling_factor;
    
    vec2 local_zero_index = vec2(gl_WorkGroupID.xy * gl_WorkGroupSize.xy * scaling_factor) / vec2(size_texutre) ;
    
    
    vec2 TexCoords = local_zero_index + vec2(gl_LocalInvocationID.xy) * single_pixel_offset;
    
    vec4 surfel_metadata = vec4(texture(surfel_framebuffer_metadata, TexCoords));
    int coverage_at_pixel = int(surfel_metadata.a);
    
    int samples_at_pixel = coverage_at_pixel>0 ? int(surfel_metadata.g) : MAX_VALUE;

    float pre = atomicMin(min_value,samples_at_pixel);
    //previous value was higher -> this is the new min 
    
    if (pre > samples_at_pixel) {
        atomicCompSwap(flatted_index_of_best_pixel, gl_LocalInvocationID.x * gl_WorkGroupSize.y + gl_LocalInvocationID.y, 0u);
        //ws_pos_of_min = texture(gPos, TexCoords).rgb;
        //level_of_min = int(vec4(texture(surfel_framebuffer_metadata, TexCoords)).b);
    }
    
    barrier();
    if (gl_LocalInvocationID.xy != uvec2(0)) {
        return;
    }
    //last thread writes value at index
    
    uint flat_index_global = gl_WorkGroupID.x * gl_NumWorkGroups.y + gl_WorkGroupID.y;

    if (min_value > MAX_SAMPLES_PER_SURFEL) {
        least_shaded_regions[flat_index_global] = vec4(-1);
    } else {

        uint x = flatted_index_of_best_pixel / gl_WorkGroupSize.y;
        uint y = flatted_index_of_best_pixel % gl_WorkGroupSize.y;


        TexCoords = local_zero_index + vec2(x,y) * single_pixel_offset;

        vec3 ws_pos_of_min = texture(gPos, TexCoords).rgb;
        int level_of_min = int(vec4(texture(surfel_framebuffer_metadata, TexCoords)).b);
        
        //level of min is set to -1 of the sampling threshold is reached 
        least_shaded_regions[flat_index_global] = vec4(ws_pos_of_min,level_of_min);
    }

}
