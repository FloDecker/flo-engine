[vertex]
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUv;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aUv;
}

[fragment]
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPos;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D dpeth_framebuffer;
uniform sampler2D gRoughnessMetallicAo;
uniform sampler2D light_map;
uniform sampler2D gSurfels;
uniform sampler2D surfel_framebuffer_metadata_0;
uniform sampler2D surfel_framebuffer_metadata_1;
uniform usampler2D gRenderFlags;
uniform sampler2D light_pass_result;


uniform sampler2D direct_light_map_texture;
layout (std140,  binding = 1) uniform DIRECT_LIGHT_UNIFORMS
{
	vec3 direct_light_direction;
	float direct_light_intensity;
	vec3 direct_light_color;
	float direct_light_light_angle;
	mat4 direct_light_light_space_matrix;
};

uniform vec3 cameraPosWs;
//surfels


#define PI 3.14159265359
#define EXPOSURE 1.5

uint bitmask_surfel_amount = 0x00FFFFFF;
float total_extension = 512.0;

float _diffuseMaterialConstant = 0.8;
float _specularMaterialConstant = 0.5;
float _ambientMaterialConstant = 0.5;

float _specularExponent = 4;

float _ambientLightIntensity = 1;

vec3 _object_color = vec3(1.0);

float bias = 0.01;
struct Surfel {
    vec4 mean_r;
    vec4 normal;
    vec4 radiance_ambient; //radiance without surface irradiance and direct light 
    vec4 radiance_direct_and_surface; //radiance contribution from direct light and surface
};


struct AllocationMetadata{
    uint surfel_bucket_pointer;
    uint surfel_octree_pointer;
    uint debug_int_32;

};


layout(std430, binding = 2) buffer AllocationMetadataBuffer {
    AllocationMetadata allocationMetadata[];
};


float random (vec2 st, float seed) {
    return fract(sin(dot(st.xy,
    vec2(12.9898, 78.233)))*
    seed);
}

vec3 float_to_heat_map (float f) {
    //return mix(
    //    vec3(235.0 / 255.0 ,64.0 / 255.0 , 52.0/ 255.0) ,
    //    vec3(41.0/ 255.0, 214.0/ 255.0, 78.0/ 255.0) ,
    //   f
    //);
    return mix(
    vec3(1, 0, 0),
    vec3(0, 1, 0),
    f
    );
}

vec2 random_vector(vec2 st, float scale) {
    float random_angle = PI * 2 * random(st, 4378.5453123f);

    return vec2(cos(random_angle), sin(random_angle))
    * random(st, 5458.24) * scale;
}

vec3 debug_bits(uint i, vec3 vertexPosWs) {
    uint t = uint(mod(int(ceil(vertexPosWs.x)), 32));
    bool s = (i & (1u << t)) != 0;
    return vec3(s, t/32.0, 0.0);
}


//post processing

float kernel[9] = float[](
1.0 / 16, 2.0 / 16, 1.0 / 16,
2.0 / 16, 4.0 / 16, 2.0 / 16,
1.0 / 16, 2.0 / 16, 1.0 / 16
);

vec3 get_distance_blur(float distance) {


    float offset = 1.0 / distance;

    vec2 offsets[9] = vec2[](
    vec2(-offset, offset),
    vec2(0.0f, offset),
    vec2(offset, offset),
    vec2(-offset, 0.0f),
    vec2(0.0f, 0.0f),
    vec2(offset, 0.0f),
    vec2(-offset, -offset),
    vec2(0.0f, -offset),
    vec2(offset, -offset)
    );

    vec3 sampleTex[9];
    for (int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(gAlbedo, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for (int i = 0; i < 9; i++) {
        col += sampleTex[i] * kernel[i];
    }
    return col;
}

vec3 gamma_correction(vec3 hdrColor) {
    const float gamma = 1.0;

    // reinhard tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * EXPOSURE);    // gamma correction 
    
    return pow(mapped, vec3(1.0 / gamma));

}

void main()
{

    //vec2 TexCoords_scaled = TexCoords * vec2(2.0f); 
     vec2 TexCoords_scaled = TexCoords; 
    
    vec3 albedo = vec3(texture(gAlbedo, TexCoords_scaled));
    vec3 normal_ws = vec3(texture(gNormal, TexCoords_scaled));
    vec3 pos_ws = vec3(texture(gPos, TexCoords_scaled));
    vec3 RoughnessMetallicAo = vec3(texture(gRoughnessMetallicAo, TexCoords_scaled));
    vec3 LightPass = vec3(texture(light_pass_result, TexCoords_scaled));
    vec4 surfel_buffer = vec4(texture(gSurfels, TexCoords_scaled));
    vec4 surfel_metadata_0 = vec4(texture(surfel_framebuffer_metadata_0, TexCoords_scaled));
    vec4 surfel_metadata_1 = vec4(texture(surfel_framebuffer_metadata_1, TexCoords_scaled));
    float depth = texture(dpeth_framebuffer, TexCoords_scaled).x;
    uint flags = texture(gRenderFlags, TexCoords_scaled).r;
    vec3 lightmap = texture(direct_light_map_texture, TexCoords_scaled).rgb;

    
    int amount_texture_fetches;
    int amount_innceseary_fetches;
   // vec3 d = get_color_from_octree(pos_ws, normal_ws, amount_texture_fetches, amount_innceseary_fetches);
    vec3 heat_map_texture_fetches = float_to_heat_map(1.0 - amount_texture_fetches * 0.01);


    uint x = allocationMetadata[0].debug_int_32;
    vec3 bit_debug = debug_bits(x, TexCoords.xxx * 128.0);
    vec3 final_color = vec3(0.0);
    LightPass = gamma_correction(LightPass);
    FragColor = vec4(LightPass, 1.0);
    return;
    if(TexCoords.y < 0.1f) {
        FragColor = vec4(bit_debug, 1.0);
        return;
    }
    
    if (TexCoords_scaled.x < 1.0) {
        if (TexCoords_scaled.y > 1.0) {

            FragColor = vec4(clamp(normal_ws.rgb, vec3(0),vec3(1)) , 1.0);
        } else {
            FragColor = vec4(surfel_metadata_0.ggg*0.01,1.0);
        }
    } else {
        if (TexCoords_scaled.y > 1.0) {
            vec3 heat_map_texture_fetches = float_to_heat_map(1.0 - surfel_metadata_0.r * 0.01);

            FragColor = vec4(heat_map_texture_fetches, 1.0);

        } else {
            FragColor = vec4(final_color, 1.0);

        }
    }

}