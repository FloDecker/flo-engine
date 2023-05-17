[vertex]

const vec3 ligtPos = 30;

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
vec3 _lightPos = vec3(0.0, 0.1, -4.0);
float _diffuseMaterialConstant = 0.3;
float _specularMaterialConstant = 0.6;
float _ambientMaterialConstant = 0.1;

float _specularExponent = 5.0;

float _ambientLightIntensity = 0.4;
float _lightIntensity = 7.0;

vec3 _reflection_vector(vec3 lightDirection) {
    return 2.0 * dot(lightDirection,normalWS) * normalWS - lightDirection;
}

float _light_intensity(vec3 lightPos) {
    return _lightIntensity/pow(distance(vertexPosWs,lightPos),2);
}

float _light_diffuse_intensity() {
    return _diffuseMaterialConstant * abs(dot(normalWS,normalize(_lightPos - vertexPosWs))) * _light_intensity(_lightPos);
}

float _light_specular_intensity() {
    return _specularMaterialConstant * 
    pow(abs(dot(
    normalize(cameraPosWs-vertexPosWs) ,
    _reflection_vector(normalize(_lightPos - vertexPosWs)))),_specularExponent)
     * _light_intensity(_lightPos);
}

float _light_ambient_intensity() {
    return _ambientLightIntensity * _ambientMaterialConstant;
}

void main() {
    //FragColor = vec4(distance(cameraPosWs,vertexPosWs)*0.1);
    FragColor = vec4(vec3(_light_diffuse_intensity() + _light_specular_intensity() + _light_ambient_intensity()),1.0);
    //FragColor = vec4(normalWS,1.0);
}