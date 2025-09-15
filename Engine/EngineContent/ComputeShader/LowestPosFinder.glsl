#version 430 core
#define MAX_SAMPLES_PER_SURFEL 10000
#define QUADRANT_SIZE 32

/*
Computeshader that finds the surfels updated least in is quadrant
Corresponds to section 4.4 in the thesis
*/

layout (local_size_x = QUADRANT_SIZE, local_size_y = QUADRANT_SIZE, local_size_z = 1) in;

layout(std430, binding = 7) buffer LeastShaded {
    vec4 least_shaded_regions[];
};

uniform sampler2D surfel_framebuffer;
uniform sampler2D surfel_framebuffer_metadata_0;
uniform sampler2D surfel_framebuffer_metadata_1;
uniform sampler2D gPos;

//variables shared within the thread group to find the minimal sampling position 
shared int min_value;
shared uint flatted_index_of_best_pixel;

bool texcoords_in_bounds(vec2 coords) {
    return coords.x >= 0.0 && coords.x <= 1.0 && coords.y >= 0.0 && coords.y <= 1.0;
}

void main() {
    bool is_first_thread = (gl_LocalInvocationID.x == 0u) && (gl_LocalInvocationID.y == 0u);
    //thread 0,0  initalizes varaibles 
    if (is_first_thread) {
        min_value  = MAX_SAMPLES_PER_SURFEL;
        flatted_index_of_best_pixel = 0u;
    }
    barrier();

    //geather screen size in pixels
    uvec2 size_texutre = uvec2(textureSize(surfel_framebuffer_metadata_0, 0).xy);

    //size of single pixel increment in screen space
    vec2 single_pixel_offset = 1.0f/vec2(size_texutre);

    //postion of the starting index for this work group in screen space
    vec2 local_zero_index = vec2(gl_WorkGroupID.xy * gl_WorkGroupSize.xy) / vec2(size_texutre);

    //screen space coordinate for this thread to work on
    vec2 TexCoords = local_zero_index + vec2(gl_LocalInvocationID.xy) * single_pixel_offset;


    //texture samples
    vec4 surfel_metadata_0 = vec4(texture(surfel_framebuffer_metadata_0, TexCoords));
    vec4 surfel_buffer = vec4(texture(surfel_framebuffer, TexCoords));

    //read the sample amout at this pixels location, if there is no surfel write MAX_VALUE
    int samples_at_pixel = surfel_buffer.a>0 && texcoords_in_bounds(TexCoords) ? int(surfel_metadata_0.w) : MAX_SAMPLES_PER_SURFEL;

    //check ccsamples_at_pixel against the current minimal sample value of the thread group 
    int pre = atomicMin(min_value, samples_at_pixel);

    //if previous value was higher, the values are swapped -> this is the new min value 
    if (pre > samples_at_pixel) {
        uint local_flatted_index = gl_LocalInvocationID.x + gl_LocalInvocationID.y * gl_WorkGroupSize.x;
        atomicExchange(flatted_index_of_best_pixel, local_flatted_index);
    }

    //wait for all threads to finish
    barrier();
    if (!is_first_thread) {
        return;
    }
    //first thread advances

    uint flat_index_global = gl_WorkGroupID.x * gl_NumWorkGroups.y + gl_WorkGroupID.y;

    uint y = flatted_index_of_best_pixel / gl_WorkGroupSize.x;
    uint x = flatted_index_of_best_pixel % gl_WorkGroupSize.x;

    TexCoords = local_zero_index + vec2(x, y) * single_pixel_offset;

    vec4 surfel_metadata_1 = vec4(texture(surfel_framebuffer_metadata_1, TexCoords));
    vec3 ws_pos_of_min = surfel_metadata_1.rgb;
    int level_of_min = int(surfel_metadata_1.w);
    //set to -1 if the sampling threshold is reached
    least_shaded_regions[flat_index_global] = (min_value < MAX_SAMPLES_PER_SURFEL)?
    vec4(ws_pos_of_min, level_of_min) :
    vec4(-1);


}
