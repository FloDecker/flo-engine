[vertex]

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;
out vec3 posWS;
out vec3 normal;
void main() {
    normal = aNormal;
    posWS =( mMatrix * vec4(aPos, 1.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos + normal*0.1, 1.0);
    gl_Position = pMatrix * vertexCamSpace; 
}

[fragment]

#version 330 core
out vec4 FragColor;
in vec3 posWS;
in vec3 normal;
void main() {
    float c = posWS.y;
    FragColor = vec4(normal + abs(normal)*1.05, 1.0f);
    //FragColor = vec4( 1.0);
}