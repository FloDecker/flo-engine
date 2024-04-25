[vertex]
out vec3 cameraPosWs;
out vec3 vertexPosWs;
out vec3 normalWS;

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
vec3 _lightPos = vec3(0.0, 0.5, -4.0);
float _diffuseMaterialConstant = 0.3;
float _specularMaterialConstant = 0.6;
float _ambientMaterialConstant = 0.1;

float _specularExponent = 16;

float _ambientLightIntensity = 0.4;
float _lightIntensity = 20.0;

vec3 _reflection_vector(vec3 lightDirection) {
    return 2.0 * dot(lightDirection,normalWS) * normalWS - lightDirection;
}

void main() {
    vec3 lightDir = normalize(_lightPos-vertexPosWs);
    vec3 viewDir = normalize(cameraPosWs-vertexPosWs);

    //light intensity
    float intensity = _lightIntensity/pow(distance(vertexPosWs,_lightPos),2);

    //diffuse
    float _light_diffuse_intensity = _diffuseMaterialConstant * max(dot(normalWS,lightDir),0.0) * intensity;
    
    //specular
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(halfDir, normalWS), 0.0);

    float diffEase = 1 - pow(1 - _light_diffuse_intensity, 3);
    float specIntensity  = pow(specAngle, _specularExponent)*diffEase; 


    float in_light = float(dot(normalWS,_lightPos - vertexPosWs) > 0);
    FragColor = vec4(vec3(
    _light_diffuse_intensity * in_light
    + specIntensity * in_light
    + _ambientLightIntensity * _ambientMaterialConstant),1.0);
}