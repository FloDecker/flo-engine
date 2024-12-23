[vertex]

out vec3 posWS;
out vec3 normal;
out vec2 texCoord;
void main() {
    texCoord = aUV;
    normal = aNormal;
    posWS =( mMatrix * vec4(aPos, 1.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace; 
}

[fragment]
in vec3 posWS;
in vec3 normal;
in vec2 texCoord;
uniform sampler2D depthMap;

void main() {
    float depthValue = texture(depthMap, texCoord).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}