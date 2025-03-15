[vertex]
out vec3 cameraPosWs;
out vec3 vertexPosWs;
out vec3 normalWS;
out vec4 FragPosLightSpace;


void main() {
    cameraPosWs = cameraPosWS;
    vertexPosWs = (mMatrix * vec4(aPos, 1.0)).xyz;
    normalWS = (transpose(inverse(mMatrix)) * vec4(aNormal, 0.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace;
}

[fragment]
#define PI 3.14159265359
in vec3 cameraPosWs;
in vec3 vertexPosWs;
in vec3 normalWS;

//static for testing
//vec3 _lightPos = vec3(0.0, 0.5, -4.0);
float _diffuseMaterialConstant = 0.6;
float _specularMaterialConstant = 0.2;
float _ambientMaterialConstant = 0.2;

float _specularExponent = 16;

float _ambientLightIntensity = 0.8;
float _lightIntensity = 10.0;

vec3 _object_color = vec3(1.0);

float bias = 0.01;


float random (vec2 st, float seed) {
    return fract(sin(dot(st.xy,
    vec2(12.9898, 78.233)))*
    seed);
}


vec2 random_vector(vec2 st, float scale) {
    float random_angle = PI * 2 * random(st, 4378.5453123f);

    return vec2(cos(random_angle), sin(random_angle))
    * random(st, 5458.24) * scale;
}

float light_map_at(vec2 coords, float mipmap) {
    float a = texture(direct_light_map_texture, coords.xy, mipmap).r;
    return a;
}

float sample_at_random(float currentDepth, vec2 coords, int samples, float distance) {
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(direct_light_map_texture, 0);

    for (int i = 0; i < samples; i++) {
        vec2 random_vec = random_vector(vertexPosWs.xy+i, 50.0* distance);
        float pcfDepth = light_map_at(coords + random_vec * texelSize, 0);
        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }

    shadow /= float(samples);
    return shadow;
}


float occluder_distance_estimation(float currentDepth, vec2 coords) {
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

    shadow /= float(kernel_widht_height * kernel_widht_height);
    return shadow;
}

bool is_inside_shadow_map_frustum() {
    vec4 frag_in_light_space = direct_light_light_space_matrix * vec4(vertexPosWs, 1.0);
    vec3 projCoords = frag_in_light_space.xyz / frag_in_light_space.w;
    projCoords = projCoords * 0.5 + 0.5;
    return (projCoords.x > 0 && projCoords.y > 0 && projCoords.x < 1 && projCoords.y < 1);
}

float in_light_map_shadow() {
    vec4 frag_in_light_space = direct_light_light_space_matrix * vec4(vertexPosWs, 1.0);
    vec3 projCoords = frag_in_light_space.xyz / frag_in_light_space.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;

    float distance = occluder_distance_estimation(currentDepth, projCoords.xy);
    if (distance <= 0) return 0;
    return sample_box(currentDepth, projCoords.xy, 7, distance*0.1);
}

vec3 _reflection_vector(vec3 lightDirection) {
    return 2.0 * dot(lightDirection, normalWS) * normalWS - lightDirection;
}

vec3 sampleColorRange(float x, vec3 color0, vec3 color1, vec3 color2,
float pos0, float pos1, float pos2) {
    // Ensure positions are sorted and x is within [0, 1]
    x = clamp(x, 0.0, 1.0);

    vec3 startColor;
    vec3 endColor;
    float startPos;
    float endPos;

    // Determine which segment x lies in
    if (x < pos1) {
        startColor = color0;
        endColor = color1;
        startPos = pos0;
        endPos = pos1;
    } else if (x < pos2) {
        startColor = color1;
        endColor = color2;
        startPos = pos1;
        endPos = pos2;
    } else {
        // Fallback in case x is slightly out of bounds due to precision
        return color2;
    }

    // Normalize x within the segment and interpolate
    float t = (x - startPos) / (endPos - startPos);
    return mix(startColor, endColor, t);
}

vec3 get_ao_color(){
    float heigt = normalWS.y * 0.5 + 0.5;
    vec3 color_mix = sampleColorRange(heigt, u_ambient_light_colors[0], u_ambient_light_colors[1], u_ambient_light_colors[2],
    u_ambient_light_colors_sample_positions[0], u_ambient_light_colors_sample_positions[1], u_ambient_light_colors_sample_positions[2]);
    return color_mix;
}

void main() {

    if (!is_inside_shadow_map_frustum()) {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        return;
    }
    vec3 lightDir = normalize(direct_light_direction);
    vec3 viewDir = normalize(cameraPosWs-vertexPosWs);

    //light intensity
    //float intensity = _lightIntensity/pow(distance(vertexPosWs,_lightPos),2);


    //diffuse
    float _light_diffuse_intensity = _diffuseMaterialConstant * max(dot(normalWS, lightDir), 0.0) * direct_light_intensity;

    //specular
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(halfDir, normalWS), 0.0);

    float diffEase = 1 - pow(1 - _light_diffuse_intensity, 3);
    float specIntensity  = pow(specAngle, _specularExponent)*diffEase;

    float test = texture(direct_light_map_texture, vertexPosWs.xy).r;

    float in_light = float(dot(normalWS, lightDir) > 0) * clamp(1.-in_light_map_shadow(), 0.0, 1.0);
    FragColor = vec4(vec3(
    _light_diffuse_intensity * in_light * _object_color * direct_light_color
    + specIntensity * in_light * direct_light_color * direct_light_intensity
    + _ambientLightIntensity * _ambientMaterialConstant * get_ao_color()), 1.0);


    ///////
    vec4 frag_in_light_space = direct_light_light_space_matrix * vec4(vertexPosWs, 1.0);
    vec3 projCoords = frag_in_light_space.xyz / frag_in_light_space.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;

    //FragColor = vec4(vec3((occluder_distance_estimation(currentDepth, projCoords.xy))),1.0);
}