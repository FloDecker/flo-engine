[vertex]
#version 420 core 

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;

flat out int test;
void main() {
    vec4 vertexCamSpace = vMatrix * mMatrix * vec4(aPos, 1.0);
    test = int(aPos.x);
    gl_Position = pMatrix * vertexCamSpace;
}

[fragment]
#version 420 core

layout(location = 0) out int object_id;

flat in int test;
void main() {
    object_id = test;
}