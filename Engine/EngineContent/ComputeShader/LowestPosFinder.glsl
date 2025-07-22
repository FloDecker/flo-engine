#version 430 core


layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

uniform sampler2D surfel_framebuffer_metadata;
uniform sampler2D gPos;

layout(std430, binding = 7) buffer LeastShaded {
    vec4 least_shaded_regions[];
};

shared int min_value;
shared vec3 ws_pos_of_min;

void main() {
    
    //thread 0,0  initalizes varaibles 
    if (gl_LocalInvocationID.xy != uvec2(0)) {
        min_value  = 5000000;
        ws_pos_of_min = vec3(0);
    }
    barrier();
    

    uvec2 size_texutre = uvec2(textureSize(surfel_framebuffer_metadata,0).xy);
    
    float scaling_factor = 4.0f;
    //4 = scaling factor 
    vec2 single_pixel_offset = 1.0f/vec2(size_texutre) * scaling_factor;
    
    vec2 local_zero_index = vec2(gl_WorkGroupID.xy * gl_WorkGroupSize.xy * scaling_factor) / vec2(size_texutre) ;
    
    
    vec2 TexCoords = local_zero_index + vec2(gl_LocalInvocationID.xy) * single_pixel_offset;
    
    int samples_at_pixel = int(vec4(texture(surfel_framebuffer_metadata, TexCoords)).w);
    
    float pre = atomicMin(min_value,samples_at_pixel);
    //previous value was higher -> this is the new min 
    
    if (pre > samples_at_pixel) {
        ws_pos_of_min = texture(gPos, TexCoords).rgb;
    }
    
    barrier();
    if (gl_LocalInvocationID.xy != uvec2(0)) {
        return;
    }
    //last thread writes value at index
    
    uint flatIndex = gl_WorkGroupID.x * gl_NumWorkGroups.y + gl_WorkGroupID.y;
    least_shaded_regions[flatIndex] = vec4(ws_pos_of_min,0);
    //least_shaded_regions[flatIndex] = vec4(local_zero_index.xyxy);//vec4(ws_pos_of_min,0);
}
