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
layout (location = 0) out vec3 final_color;

in vec2 TexCoords;

uniform sampler2D gPos;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gEmissive;
uniform sampler2D dpeth_framebuffer;
uniform sampler2D gRoughnessMetallicAo;
uniform sampler2D light_pass_result;
uniform sampler2D gSurfels;
uniform sampler2D surfel_framebuffer_metadata_0;
uniform sampler2D surfel_framebuffer_metadata_1;
uniform usampler2D gRenderFlags;


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
#define AREA_UNIT_SPHERE (4.0*PI)
#define FLOAT_MAX 200000.0
#define GAUSSIAN_SAMPLES 512

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



vec3 _reflection_vector(vec3 lightDirection, vec3 normalWS) {
    return 2.0 * dot(lightDirection, normalWS) * normalWS - lightDirection;
}

vec3 projectPointOnPlane(vec3 P, vec3 P0, vec3 N) {
    N = normalize(N);// Ensure normal is unit length

    float d = dot(P - P0, N);
    return P - d * N;// Projected point
}

float rayPlaneIntersection(vec3 rayOrigin, vec3 rayDir, vec3 planeNormal, vec3 planePoint) {
    float denom = dot(planeNormal, rayDir);
    if (abs(denom) > 1e-6) { // Avoid division by zero
        float t = dot(planeNormal, planePoint - rayOrigin) / denom;
        return (t >= 0.0) ? t : -1.0;// Only return valid intersections
    }
    return -1.0;// No intersection or parallel
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

vec3 get_ao_color(){
    return vec3(80.0/255.0,156.0/255.0,250.0/255.0) * 0.3f;
}

float in_light_map_shadow() {
    return 0;
    
}

//lightmapping

float light_map_at(vec2 coords, float mipmap) {
    float a = texture(direct_light_map_texture, coords.xy, mipmap).r;
    return a;
}

float sample_box(float currentDepth, vec2 coords, int kernel_widht_height, float distance){
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(direct_light_map_texture, 0);
    int kernel_widht_height_half = (kernel_widht_height - 1)/ 2;
    for (int x = -kernel_widht_height_half; x <= kernel_widht_height_half; x++) {
        for (int y = -kernel_widht_height_half; y < kernel_widht_height_half; y++) {
            vec2 offset_vec = vec2(x, y);
            float pcfDepth = light_map_at(coords + offset_vec * texelSize * 0.5, 0);
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    shadow /= float((kernel_widht_height - 1)  * (kernel_widht_height - 1));
    return shadow;
}

bool is_inside_shadow_map_frustum(vec3 vertexPosWs) {
    vec4 frag_in_light_space = direct_light_light_space_matrix * vec4(vertexPosWs, 1.0);
    vec3 projCoords = frag_in_light_space.xyz / frag_in_light_space.w;
    projCoords = projCoords * 0.5 + 0.5;
    return (projCoords.x > 0 && projCoords.y > 0 && projCoords.x < 1 && projCoords.y < 1);
}

float occluder_distance_estimation(float currentDepth, vec2 coords, vec3 vertexPosWs) {
    int kernel_widht_height = 2;
    vec2 texelSize = 1.0 / textureSize(direct_light_map_texture, 0);

    float avg_distance = 0.0;
    float ocluder_hits = 0.0;
    float max_distance = 0.0;
    for (int x = -kernel_widht_height; x < kernel_widht_height; x++) {
        for (int y = -kernel_widht_height; y < kernel_widht_height; y++) {
            vec2 random_vec = random_vector(vertexPosWs.xy, 1);

            vec2 offset_vec = vec2(x, y);// + random_vec;

            float pcfDepth = light_map_at(coords + offset_vec*texelSize, 0);
            if (currentDepth - bias > pcfDepth) {
                ocluder_hits+=1;
                avg_distance+= (currentDepth - pcfDepth);
            }

        }
    }

    return avg_distance/(kernel_widht_height*kernel_widht_height);
}

float in_light_map_shadow(vec3 vertexPosWs) {
    vec4 frag_in_light_space = direct_light_light_space_matrix * vec4(vertexPosWs, 1.0);
    vec3 projCoords = frag_in_light_space.xyz / frag_in_light_space.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    float pcfDepth = light_map_at(projCoords.xy, 0);
    if(pcfDepth == 1) return 0;
    float currentDepth = projCoords.z;

    float distance = occluder_distance_estimation(currentDepth, projCoords.xy,vertexPosWs);
    if (distance <= 0) return 0;
    return sample_box(currentDepth, projCoords.xy, 7, distance*0.1);
}

/////////////////

vec3 phong(vec3 vertexPosWs, vec3 normalWS, vec3 albedo, vec3 surfel_buffer, bool has_surfel){
    vec3 lightDir = normalize(direct_light_direction);
    vec3 viewDir = normalize(cameraPosWs-vertexPosWs);

    //light intensity
    //float intensity = _lightIntensity/pow(distance(vertexPosWs,_lightPos),2);


    //diffuse
    
    float in_light = float(dot(normalWS, lightDir) > 0)
    * (is_inside_shadow_map_frustum(vertexPosWs)? clamp(1.-in_light_map_shadow(vertexPosWs), 0.0, 1.0) : 1.0); 
    

    float _light_diffuse_intensity = _diffuseMaterialConstant * max(dot(normalWS, lightDir), 0.0) * direct_light_intensity;
    
    //specular
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(halfDir, normalWS), 0.0);

    float diffEase = 1 - pow(1 - _light_diffuse_intensity, 3);
    float specIntensity = clamp(pow(specAngle, _specularExponent)*diffEase,0,1);
    
    vec3 diffuse_part = _light_diffuse_intensity * in_light * albedo * direct_light_color;
    vec3 specular_part = specIntensity * in_light * direct_light_color * direct_light_intensity;
    vec3 ambient_part = _ambientLightIntensity * _ambientMaterialConstant * (has_surfel ? surfel_buffer:get_ao_color());

    return diffuse_part + specular_part + ambient_part;
}

//from https://learnopengl.com/PBR/Theory
float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 calculate_pbr_lighting (vec3 WorldPos, vec3 Normal, vec3 albedo, float roughness, float metallic, float ao, vec3 emissive, vec4 surfel_buffer) {
    vec3 N = normalize(Normal);
    vec3 V = normalize(cameraPosWs - WorldPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    // calculate per-light radiance
    vec3 L = normalize(direct_light_direction);
    vec3 H = normalize(V + L);
    vec3 radiance  = direct_light_color * direct_light_intensity;
    
    //calcualte shadows
    float in_light = float(dot(Normal, L) > 0)
    * (is_inside_shadow_map_frustum(WorldPos)? clamp(1.-in_light_map_shadow(WorldPos), 0.0, 1.0) : 1.0);
    
    radiance*=in_light;

    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator    = F * NDF * G;
    float denominator = max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    
    
    vec3 ambient = (vec3(0.00) * albedo + 0.9f*clamp(surfel_buffer.rgb, vec3(0.0), vec3(1.0)))* ao;
    vec3 color = ambient + Lo + emissive;
    return vec3( color);
}

void main()
{

    //vec2 TexCoords_scaled = TexCoords * vec2(2.0f); 
     vec2 TexCoords_scaled = TexCoords; 
    
    vec3 albedo = texture(gAlbedo, TexCoords_scaled).rgb;
    vec3 normal_ws = vec3(texture(gNormal, TexCoords_scaled));
    vec3 pos_ws = vec3(texture(gPos, TexCoords_scaled));
    vec3 emissive = vec3(texture(gEmissive, TexCoords_scaled));
    vec3 RoughnessMetallicAo = vec3(texture(gRoughnessMetallicAo, TexCoords_scaled));
    vec4 surfel_buffer = vec4(texture(gSurfels, TexCoords_scaled));
    float depth = texture(dpeth_framebuffer, TexCoords_scaled).x;
    uint flags = texture(gRenderFlags, TexCoords_scaled).r;
    vec3 lightmap = texture(direct_light_map_texture, TexCoords_scaled).rgb;
    

    final_color = vec3(0.0);
    if (flags == 0) {
        //final_color = phong(pos_ws, normal_ws, albedo, surfel_buffer.rgb, surfel_buffer.w > 0);
        //final_color = calculate_pbr_lighting(pos_ws, normal_ws, albedo, RoughnessMetallicAo.r, RoughnessMetallicAo.g, RoughnessMetallicAo.b, surfel_buffer);
        final_color = calculate_pbr_lighting(pos_ws, normal_ws, albedo, 0.5, 0.0, 1.0,emissive, surfel_buffer);
    } else {
        final_color = albedo ;
    }
    
}