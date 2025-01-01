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

float in_light_map_shadow() {
    vec4 frag_in_light_space = direct_light_light_space_matrix * vec4(vertexPosWs, 1.0);
    vec3 projCoords = frag_in_light_space.xyz / frag_in_light_space.w;
    projCoords = projCoords * 0.5 + 0.5;

    float bias = 0.01;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(direct_light_map_texture, 0);
    float currentDepth = projCoords.z;
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(direct_light_map_texture, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    return shadow;
}

vec3 _reflection_vector(vec3 lightDirection) {
    return 2.0 * dot(lightDirection,normalWS) * normalWS - lightDirection;
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
    vec3 color_mix = sampleColorRange(heigt,u_ambient_light_colors[0] , u_ambient_light_colors[1] , u_ambient_light_colors[2],
    u_ambient_light_colors_sample_positions[0], u_ambient_light_colors_sample_positions[1], u_ambient_light_colors_sample_positions[2]);
    return color_mix;
}

void main() {
    vec3 lightDir = normalize(direct_light_direction);
    vec3 viewDir = normalize(cameraPosWs-vertexPosWs);

    //light intensity
    //float intensity = _lightIntensity/pow(distance(vertexPosWs,_lightPos),2);
    
    
    //diffuse
    float _light_diffuse_intensity = _diffuseMaterialConstant * max(dot(normalWS,lightDir),0.0) * direct_light_intensity;
    
    //specular
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(halfDir, normalWS), 0.0);

    float diffEase = 1 - pow(1 - _light_diffuse_intensity, 3);
    float specIntensity  = pow(specAngle, _specularExponent)*diffEase; 

    float test = texture(direct_light_map_texture,vertexPosWs.xy).r;
    
    float in_light = float(dot(normalWS,lightDir) > 0) * (1-in_light_map_shadow());
    FragColor = vec4(vec3(
    _light_diffuse_intensity * in_light * _object_color * direct_light_color
    + specIntensity * in_light * direct_light_color * direct_light_intensity
    + _ambientLightIntensity * _ambientMaterialConstant * get_ao_color()),1.0);
}