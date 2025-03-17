[vertex]
out vec3 pos_ws;
void main() {
    pos_ws = (mMatrix * vec4(aPos, 1.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace;
}

[fragment]
in vec3 pos_ws;
void main() {
    FragColor = vec4(pos_ws, 1.0);
}