[vertex]

flat out vec3 normal_ws;

void main() {
    normal_ws = (mMatrix * vec4(aNormal, 0.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace;
}

[fragment]
flat in vec3 normal_ws;

void main() {
    FragColor = vec4(normal_ws, 1.0);
}